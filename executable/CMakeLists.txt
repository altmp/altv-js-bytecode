project("bytecode-executable")

include_directories(
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/deps"
    "${GLOBAL_DEPS_FOLDER}/cpp-sdk/deps"
    "${GLOBAL_DEPS_FOLDER}/v8/include"
    "${GLOBAL_COMPILER_PATH}"
)

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.h" "src/*.cpp")

include(GroupSources)
GroupSources(${PROJECT_SOURCE_DIR}/src "Source Files")

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_FILES}
)

if(UNIX)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fPIC
    )
endif()

include(Shared)
SetupProject("executable")

set(ALTV_JS_DEFS
    # Compliation
    -DCXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}"
    # Platform
    -DUNICODE
    -D_UNICODE
    # v8
    -DV8_COMPRESS_POINTERS
    -DV8_31BIT_SMIS_ON_64BIT_ARCH
    -DV8_IMMINENT_DEPRECATION_WARNINGS
  )
target_compile_definitions(${PROJECT_NAME} PRIVATE
  ${ALTV_JS_DEFS}
)

if(WIN32)
    set(LINK_LIBS
        Winmm.lib
        DbgHelp.lib
        shlwapi.lib
        ${GLOBAL_DEPS_FOLDER}/v8/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/v8_monolith.lib
    )
elseif(UNIX)
    set(LINK_LIBS
        ${GLOBAL_DEPS_FOLDER}/v8/lib/libv8_monolith.a
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    compiler
    ${LINK_LIBS}
)
