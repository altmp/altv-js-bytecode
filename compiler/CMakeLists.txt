project("compiler")

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${GLOBAL_DEPS_FOLDER}/v8/include"
)

include(GroupSources)
GroupSources(${PROJECT_SOURCE_DIR} "Source Files")

add_library(${PROJECT_NAME} STATIC "compiler.cpp" "compiler.h")

if(UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -std=c++17
        -stdlib=libstdc++
        -fPIC
    )
endif()

include(Shared)
SetupProject("compiler")
