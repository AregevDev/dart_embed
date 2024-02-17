cmake_minimum_required(VERSION 3.27)
project(dart_embed CXX)

set(CMAKE_CXX_STANDARD 11)
set(DART_SDK_DIR C:\\Users\\Oak\\Documents\\Git\\dart_shared_libray\\dart-sdk\\sdk)

find_library(LIB_DART_DEBUG
        NAMES "libdart"
        HINTS "${DART_SDK_DIR}/out/DebugX64/obj/runtime/bin" "${DART_SDK_DIR}/xcodebuild/ReleaseX64/obj/runtime/bin"
)

find_library(LIB_DART_RELEASE
        NAMES "libdart"
        HINTS "${DART_SDK_DIR}/out/ReleaseX64/obj/runtime/bin" "${DART_SDK_DIR}/xcodebuild/ReleaseX64/obj/runtime/bin"
)

add_library(dart_embed STATIC
        src/setup.cpp)

set_property(TARGET dart_embed PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(dart_embed PUBLIC ${DART_SDK_DIR}/runtime)
target_link_libraries(dart_embed PUBLIC ${LIB_DART_RELEASE} dbghelp psapi rpcrt4 shlwapi iphlpapi bcrypt)

add_executable(dart_embed_cmd cmd/main.cpp)
target_include_directories(dart_embed_cmd PRIVATE include)
target_link_libraries(dart_embed_cmd PRIVATE dart_embed)
