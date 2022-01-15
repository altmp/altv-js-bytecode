project("js-bytecode-module")

set(DEPS_FOLDER ${PROJECT_SOURCE_DIR}/deps)

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.h" "src/*.cpp")

include(GroupSources)
GroupSources(${PROJECT_SOURCE_DIR}/src "Source Files")
GroupSources(${GLOBAL_COMPILER_PATH} "Compiler Files")

include_directories(
  "${PROJECT_SOURCE_DIR}/deps"
  "${GLOBAL_DEPS_FOLDER}/v8/include"
  "${GLOBAL_COMPILER_PATH}"
)

add_library(${PROJECT_NAME} SHARED
  ${PROJECT_SOURCE_FILES}
)

include(Shared)
SetupProject("module")

set(ALTV_JS_DEFS
    # Compliation
    -DCXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}"
    # Platform
    -DUNICODE
    -D_UNICODE
    # alt:V
    -DALT_SERVER_API
    # v8
    -DV8_COMPRESS_POINTERS
    -DV8_31BIT_SMIS_ON_64BIT_ARCH
    -DV8_IMMINENT_DEPRECATION_WARNINGS
  )
target_compile_definitions(${PROJECT_NAME} PRIVATE
  ${ALTV_JS_DEFS}
)

if(WIN32)
  set(ALTV_JS_LINKS
    # Platform binaries
    Winmm.lib
    DbgHelp.lib
    shlwapi.lib

    # V8
    ${GLOBAL_DEPS_FOLDER}/v8/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/v8_monolith.lib
  )
elseif(UNIX)
  set(ALTV_JS_LINKS
    # V8
    ${GLOBAL_DEPS_FOLDER}/v8/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/v8_monolith.so
  )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
  compiler
  ${ALTV_JS_LINKS}
)
