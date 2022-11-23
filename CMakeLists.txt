cmake_minimum_required (VERSION 3.10)

project("js-bytecode")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(GLOBAL_DEPS_FOLDER "${CMAKE_SOURCE_DIR}/deps")
set(GLOBAL_COMPILER_PATH "${CMAKE_SOURCE_DIR}/compiler")

include(DepsDownload)
message("alt:V JS Bytecode - Fetching v8 deps, can take a while")
DownloadDeps(${GLOBAL_DEPS_FOLDER})

if(WIN32 AND MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /Zi /bigobj")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /Zi /bigobj")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG:FULL /OPT:REF /OPT:ICF")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG:FULL /OPT:REF /OPT:ICF")
    add_definitions(/MP)
endif()

add_subdirectory("compiler")
add_subdirectory("module")
# WIP
# add_subdirectory("executable")
