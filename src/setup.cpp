//
// Created by Oak on 2/17/2024.
//

#include <include/dart_api.h>
#include <include/dart_embedder_api.h>
#include <bin/dfe.h>
#include <bin/dartutils.h>

#include <iostream>

using dart::bin::DartUtils;
using dart::bin::dfe;

extern "C"
{
extern const uint8_t kDartVmSnapshotData[];
extern const uint8_t kDartVmSnapshotInstructions[];
}

/// Dart VM Initialization Config
typedef struct DartInitConfig
{
    int argc;
    const char *argv;
    Dart_InitializeParams params;
} DartInitConfig;

int dart_embed_init(DartInitConfig config, int argc, const char **argv)
{
    Dart_SetVMFlags(argc, argv);

    char *embedder_init_error = nullptr;
    if (dart::embedder::InitOnce(&embedder_init_error) && embedder_init_error)
    {
        // Should we leave the error logging to the end user?
        std::cerr << "dart::embedder::InitOnce failed: " << embedder_init_error << '\n';
        return 1;
    }

    // Force default values
    config.params.version = DART_INITIALIZE_PARAMS_CURRENT_VERSION;
    config.params.vm_snapshot_data = kDartVmSnapshotData;
    config.params.vm_snapshot_instructions = kDartVmSnapshotInstructions;
    config.params.start_kernel_isolate = dfe.CanUseDartFrontend() && dfe.UseDartFrontend();

    dfe.Init();
    dfe.set_use_dfe();
    dfe.set_use_incremental_compiler(true);

    char *dart_init_error = Dart_Initialize(&config.params);
    if (dart_init_error)
    {
        std::cerr << "Dart_Initialize failed: " << dart_init_error << '\n';
        return 1;
    }

    delete dart_init_error;
    return 0;
}

void dart_embed_cleanup()
{
    Dart_Cleanup();
    dart::embedder::Cleanup();
}

void run_test()
{
    DartInitConfig config = {0};

    dart_embed_init(config, 0, nullptr);
    std::cout << "Dart VM version: " << Dart_VersionString() << '\n';
    dart_embed_cleanup();
}
