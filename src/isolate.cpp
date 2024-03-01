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

void setup_core_libraries(Dart_Isolate isolate)
{

    DartUtils::PrepareForScriptLoading(false, true);
}

void dart_embed_create_isolate(Dart_Isolate isolate)
{

}
