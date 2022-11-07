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

if(WIN32)
    set(LINK_LIBS
        Winmm.lib
        DbgHelp.lib
        shlwapi.lib
        ${GLOBAL_DEPS_FOLDER}/v8/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/v8_monolith.lib
    )
elseif(UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -std=c++17
        -stdlib=libstdc++
    )
    set(LINK_LIBS
        ${GLOBAL_DEPS_FOLDER}/v8/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/libv8_monolith.a
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    compiler
    ${LINK_LIBS}
)
