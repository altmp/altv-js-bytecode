cmake_minimum_required (VERSION 3.10)
include(FetchContent)
include(cmake/DepsDownload.cmake)

project("js-bytecode-module")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DEPS_FOLDER ${PROJECT_SOURCE_DIR}/deps)

# Fetch v8
message("alt:V JS - Fetching v8 deps, can take a while")

DownloadDeps(${DEPS_FOLDER})

file(GLOB_RECURSE PROJECT_SOURCE_FILES "src/*.h" "src/*.cpp")
file(GLOB_RECURSE COMPILER_SOURCE_FILES "../compiler/*.h" "../compiler/*.cpp")

macro(GroupSources curdir groupindex)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)

  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      GroupSources(${curdir}/${child} ${groupindex}/${child})
    else()

      string(REPLACE "/" "\\" groupname ${groupindex})

      source_group(${groupname} FILES ${curdir}/${child})
    endif()
  endforeach()
endmacro()

GroupSources(${PROJECT_SOURCE_DIR}/src "Source Files")
GroupSources(${PROJECT_SOURCE_DIR}/../compiler "Compiler Files")

include_directories(
  "deps"
  "deps/v8/include"
  "../compiler"
)

add_library(
  ${PROJECT_NAME} SHARED
  ${PROJECT_SOURCE_FILES}
  ${COMPILER_SOURCE_FILES}
)

if(WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD /Zi /bigobj")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /bigobj")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG:FULL /OPT:REF /OPT:ICF")
endif()

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
    ${DEPS_FOLDER}/v8/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/v8_monolith.lib
  )
elseif(UNIX)
  set(ALTV_JS_LINKS
    # V8
    ${DEPS_FOLDER}/v8/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/v8_monolith.so
  )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${ALTV_JS_LINKS}
)
