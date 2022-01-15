project("compiler")

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${GLOBAL_DEPS_FOLDER}/v8/include"
)

include(GroupSources)
GroupSources(${PROJECT_SOURCE_DIR} "Source Files")

add_library(${PROJECT_NAME} STATIC "compiler.cpp" "compiler.h")

include(Shared)
SetupProject("compiler")
