project("bytecode-executable")

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${GLOBAL_DEPS_FOLDER}/v8/include"
)

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.h" "src/*.cpp")

include(GroupSources)
GroupSources(${PROJECT_SOURCE_DIR}/src "Source Files")

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_FILES}
)

include(Shared)
SetupProject("executable")
