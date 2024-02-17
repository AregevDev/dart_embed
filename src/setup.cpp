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

extern "C" {
extern const uint8_t kDartVmSnapshotData[];
extern const uint8_t kDartVmSnapshotInstructions[];
}

void run_test() {
    Dart_SetVMFlags(0, nullptr);

    dart::embedder::InitOnce(nullptr);

    Dart_InitializeParams params = {0};
    params.version = DART_INITIALIZE_PARAMS_CURRENT_VERSION;
    params.vm_snapshot_data = kDartVmSnapshotData;
    params.vm_snapshot_instructions = kDartVmSnapshotInstructions;
    params.start_kernel_isolate = dfe.CanUseDartFrontend() && dfe.UseDartFrontend();

    dfe.Init();
    dfe.set_use_dfe();
    dfe.set_use_incremental_compiler(true);

    char *init_error = Dart_Initialize(&params);
    if (init_error) {
        std::cout << init_error << '\n';
    }

    std::cout << "Dart VM version: " << Dart_VersionString() << '\n';

    Dart_Cleanup();
    dart::embedder::Cleanup();

    // delete init_error;
}
