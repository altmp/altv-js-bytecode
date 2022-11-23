project("compiler")

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${GLOBAL_DEPS_FOLDER}/v8/include"
)

include(GroupSources)
GroupSources(${PROJECT_SOURCE_DIR} "Source Files")

add_library(${PROJECT_NAME} STATIC "compiler.cpp")

if(UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fPIC
    )
endif()

include(Shared)
SetupProject("compiler")
