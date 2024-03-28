#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "include/dart_api.h"
#include "include/dart_embedder_api.h"

#include "bin/dartutils.h"
#include "bin/isolate_data.h"

#include "incbin/incbin.h"
// Linked snapshots provide fast way to spawn new isolates.
// Important: if when spawning isolate from core snapshot you need to
// pass VmSnapshot to Dart_Initialize() via vm_snapshot_data and
// vm_snapshot_instructions parameters.

extern "C" {
extern const uint8_t kDartVmSnapshotData[];
extern const uint8_t kDartVmSnapshotInstructions[];
extern const uint8_t kDartCoreIsolateSnapshotData[];
extern const uint8_t kDartCoreIsolateSnapshotInstructions[];
extern const uint8_t kPlatformStrongDill[];
extern intptr_t kPlatformStrongDillSize;
extern const uint8_t kKernelServiceDill[];
extern intptr_t kKernelServiceDillSize;

INCBIN(TestDart, "kernel/test.dill");
}

namespace {
    bool ReadFile(const char* path, std::vector<char>& buf) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        buf.resize(size);
        return !file.read(buf.data(), size).fail();
    }

#define ABORT_IF_ERROR(e) AbortIfError((e), #e)
#define ABORT_IF_ERROR_OR_NULL(e) AbortIfErrorOrNull((e), #e)

    const char* ToCString(Dart_Handle handle) {
        if (Dart_IsError(handle)) {
            return Dart_GetError(handle);
        } else if (Dart_IsString(handle)) {
            const char* result = nullptr;
            Dart_Handle err = Dart_StringToCString(handle, &result);
            if (Dart_IsError(err)) {
                return Dart_GetError(err);
            } else {
                return result;
            }
        } else {
            return ToCString(Dart_ToString(handle));
        }
    }

    Dart_Handle AbortIfError(Dart_Handle result, const char* action) {
        if (Dart_IsError(result)) {
            std::cerr << action << " failed with error: " << ToCString(result)
                      << std::endl;
            abort();
        }
        return result;
    }

    Dart_Handle AbortIfErrorOrNull(Dart_Handle result, const char* action) {
        if (Dart_IsNull(AbortIfError(result, action))) {
            std::cerr << action << " returned null" << std::endl;
            abort();
        }
        return result;
    }

    Dart_Handle NewString(const char* str) {
        return ABORT_IF_ERROR(Dart_NewStringFromCString(str));
    }

    std::string kernel_service_dill_path;
    std::vector<char> kernel_service_dill;

    static Dart_Isolate CreateIsolateGroupAndSetup(const char* script_uri,
                                                   const char* main,
                                                   const char* package_root,
                                                   const char* package_config,
                                                   Dart_IsolateFlags* flags,
                                                   void* callback_data,
                                                   char** error) {

//        flags->null_safety = true;
//
//        Dart_Isolate isolate = Dart_CreateIsolateGroupFromKernel(
//                DART_KERNEL_ISOLATE_NAME, DART_KERNEL_ISOLATE_NAME,
//                kKernelServiceDill,
//                kKernelServiceDillSize, flags,
//                /*isolate_group_data=*/nullptr, /*isolate_data=*/nullptr, error);
//        if (isolate == nullptr) {
//            std::cerr << "error on line " << __LINE__ << " " << *error << std::endl;
//            return nullptr;
//        }
//
//        Dart_EnterScope();
//
//        ABORT_IF_ERROR(dart::bin::DartUtils::PrepareForScriptLoading(
//                /*is_service_isolate=*/false, /*trace_loading=*/false));
//        ABORT_IF_ERROR(Dart_LoadScriptFromKernel(
//                kKernelServiceDill, kKernelServiceDillSize));
//        Dart_ExitScope();
//        Dart_ExitIsolate();
//        *error = Dart_IsolateMakeRunnable(isolate);
//        if (*error != nullptr) {
//            std::cerr << "error " << *error << std::endl;
//            Dart_EnterIsolate(isolate);
//            Dart_ShutdownIsolate();
//            return nullptr;
//        }
//
//        return isolate;

        return nullptr;
    }

}  // namespace

// This is almost the minimal example of Dart runtime embedding. In this example
// we don't start either kernel-service or vm-service isolates and don't use
// kernel platform or core snapshot to initialize the isolate, instead we
// expect fully linked kernel program as input.
//
//   $ ninja -C out/ReleaseX64 embedder_example_3 vm_platform
//   $ cat /tmp/hello.dart
//   import 'dart:async';
//
//   void main() {
//     print("Hello, World!");
//     var count = 0;
//     Timer.periodic(Duration(seconds: 1), (timer) {
//       print('${++count} second${count > 1 ? 's' : ''} passed');
//       if (count > 5) {
//         timer.cancel();
//       }
//     });
//   }
//   $ dart pkg/vm/bin/gen_kernel.dart --platform out/ReleaseX64/vm_platform_strong.dill -o /tmp/hello.dill /tmp/hello.dart
//   $ out/ReleaseX64/embedder_example_3  /tmp/kernel-service.dill out/ReleaseX64/vm_platform_strong.dill /tmp/hello.dart
//   Hello, World!
//   1 second passed
//   2 seconds passed
//   3 seconds passed
//   4 seconds passed
//   5 seconds passed
//   6 seconds passed
//
void run_test()
{
    const char *dart_source_path = "kernel/test.dart";

    char *error = nullptr;
    if (!dart::embedder::InitOnce(&error)) {
        std::cerr << "Standalone embedder initialization failed: " << error
                  << std::endl;
        free(error);
        abort();
    }

    std::vector<const char *> flags{};
    error = Dart_SetVMFlags(flags.size(), flags.data());
    if (error != nullptr) {
        std::cerr << "Dart_SetVMFlags failed: " << error << std::endl;
        abort();
    }

    Dart_InitializeParams init_params;
    memset(&init_params, 0, sizeof(init_params));
    init_params.vm_snapshot_data = kDartVmSnapshotData;
    init_params.vm_snapshot_instructions = kDartVmSnapshotInstructions;
    init_params.create_group = CreateIsolateGroupAndSetup;
    init_params.start_kernel_isolate = true;
    init_params.version = DART_INITIALIZE_PARAMS_CURRENT_VERSION;

    error = Dart_Initialize(&init_params);
    if (error != nullptr) {
        dart::embedder::Cleanup();
        std::cerr << "Dart_Initialize failed: " << error << std::endl;
        free(error);
        abort();
    }

    uint8_t *kernel_buffer;
    intptr_t kernel_buffer_size;

//    auto result = Dart_CompileToKernel(
//            dart_source_path,
//            kPlatformStrongDill,
//            kPlatformStrongDillSize, /*incremental=*/false,
//            false,
//            true,
//            /*package_config=*/nullptr,
//            Dart_KernelCompilationVerbosityLevel_All);
//
//    switch (result.status) {
//        case Dart_KernelCompilationStatus_Ok:
//            kernel_buffer = result.kernel;
//            kernel_buffer_size = result.kernel_size;
//            break;
//        case Dart_KernelCompilationStatus_Error:
//            free(result.kernel);
//            std::cerr << "Failed to compile " << dart_source_path << std::endl
//                      << result.error << std::endl;
//            abort();
//            break;
//        case Dart_KernelCompilationStatus_Crash:
//            free(result.kernel);
//            std::cerr << "Failed to compile " << dart_source_path
//                      << " (compiler crashed)" << std::endl
//                      << result.error << std::endl;
//            abort();
//            break;
//        case Dart_KernelCompilationStatus_Unknown:
//            free(result.kernel);
//            std::cerr << "Failed to compile " << dart_source_path
//                      << " (unknown reason)" << std::endl
//                      << result.error << std::endl;
//            abort();
//            break;
//    }

    Dart_IsolateFlags flags1{};
    flags1.null_safety = true;

    // Ready to create and run isolates here.
    Dart_Isolate isolate = Dart_CreateIsolateGroup(
            /*script_uri=*/dart_source_path,
            /*name=*/"main",
            /*isolate_snapshot_data=*/kDartCoreIsolateSnapshotData,
            /*isolate_snapshot_instructions=*/kDartCoreIsolateSnapshotInstructions,
            /*flags=*/&flags1,
            /*isolate_group_data=*/nullptr,
            /*isolate_data=*/nullptr, &error);

    if (isolate == nullptr) {
        std::cerr << "Failed to create an isolate: " << error << std::endl;
        abort();
    }

    Dart_EnterScope();

    // Initialize core libraries.
    ABORT_IF_ERROR(dart::bin::DartUtils::PrepareForScriptLoading(
            /*is_service_isolate=*/false, /*trace_loading=*/false));

    // Dart_CreateIsolateGroupFromKernel currently does not properly set the root
    // library (one that contains main entry point) so we are just supplying
    // it from the command line.
    Dart_Handle main_library = ABORT_IF_ERROR_OR_NULL(
            Dart_LoadLibraryFromKernel(gTestDartData, gTestDartSize));
    ABORT_IF_ERROR(Dart_SetRootLibrary(main_library));

    // Have a main isolate ready to go - we are in it after Dart_CreateIsolate*.
    Dart_Handle root_library = ABORT_IF_ERROR_OR_NULL(Dart_RootLibrary());

    ABORT_IF_ERROR(Dart_Invoke(root_library, NewString("main"), 0, nullptr));
    ABORT_IF_ERROR(Dart_RunLoop());

    Dart_ExitScope();
    Dart_ShutdownIsolate();

    error = Dart_Cleanup();
    if (error != nullptr) {
        std::cerr << "Dart_Cleanup failed: " << error << std::endl;
        free(error);
    }

    dart::embedder::Cleanup();
}
