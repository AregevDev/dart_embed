cmake_minimum_required(VERSION 3.27)
project(dart_embed CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(DART_EMBED_SRC
        src/setup.cpp
)

# CMake needs all of it's source files present at configuration time.
set(INCBIN_ASSET_FILENAME data.c)
file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/${INCBIN_ASSET_FILENAME})

set(DART_SDK_DIR C:\\Users\\Oak\\Documents\\Git\\dart_shared_libray\\dart-sdk\\sdk)
find_library(LIB_DART_DEBUG
        NAMES "libdart"
        HINTS "${DART_SDK_DIR}/out/DebugX64/obj/runtime/bin" "${DART_SDK_DIR}/xcodebuild/ReleaseX64/obj/runtime/bin"
)

find_library(LIB_DART_RELEASE
        NAMES "libdart"
        HINTS "${DART_SDK_DIR}/out/ReleaseX64/obj/runtime/bin" "${DART_SDK_DIR}/xcodebuild/ReleaseX64/obj/runtime/bin"
)

# Build incbin from source and copy the header
add_executable(incbin extern/incbin/incbin.c)
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/incbin/incbin.h
        ${CMAKE_CURRENT_BINARY_DIR}/incbin.h COPYONLY
)

# Run the command for every source file change
add_custom_target(include_incbin_data ALL
        COMMAND incbin -o ${CMAKE_CURRENT_BINARY_DIR}/${INCBIN_ASSET_FILENAME} ${DART_EMBED_SRC}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${DART_EMBED_SRC})

add_dependencies(include_incbin_data incbin)

add_library(dart_embed STATIC
        ${DART_EMBED_SRC}
        ${CMAKE_CURRENT_BINARY_DIR}/${INCBIN_ASSET_FILENAME}
        )

# Use static CRT
set_property(TARGET dart_embed PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_dependencies(dart_embed include_incbin_data)
target_include_directories(dart_embed PUBLIC ${DART_SDK_DIR}/runtime)
target_include_directories(dart_embed PRIVATE extern)
target_link_libraries(dart_embed PUBLIC ${LIB_DART_RELEASE} dbghelp psapi rpcrt4 shlwapi iphlpapi bcrypt)

add_executable(dart_embed_cmd cmd/main.cpp)
target_include_directories(dart_embed_cmd PRIVATE include)
target_link_libraries(dart_embed_cmd PRIVATE dart_embed)
