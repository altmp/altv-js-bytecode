project("bytecode-executable")

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/deps"
    "${GLOBAL_DEPS_FOLDER}/v8/include"
    "${GLOBAL_COMPILER_PATH}"
)

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.h" "src/*.cpp")

include(GroupSources)
GroupSources(${PROJECT_SOURCE_DIR}/src "Source Files")

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_FILES}
)

include(Shared)
SetupProject("executable")
