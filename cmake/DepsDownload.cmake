include(./cmake/DepsHelpers.cmake)

# Set this to false, when using a custom v8 build for testing
set(__deps_check_enabled true)
set(V8_VERSION "12.4.254")

function(DownloadDeps)
    set(__base_path "${PROJECT_SOURCE_DIR}/deps/v8")

    GetBranchAndOS(__deps_branch __deps_os_path_name)
    set(__deps_url_base_path "https://cdn.alt-mp.com/deps/v8/${V8_VERSION}")

    if(__deps_check_enabled)
        if(WIN32)
            message("Checking release binaries...")

            GetCDNInfo("${__deps_url_base_path}/${__deps_os_path_name}/Release" __deps_release_hashes __deps_current_version)
            DownloadFile("v8_monolith.lib" "${__base_path}/lib/Release" "${__deps_os_path_name}/Release" ${__deps_release_hashes})

            # Only download debug binary in Debug builds
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                message("Checking debug binaries...")

                # GetCDNInfo("${__deps_url_base_path}/${__deps_os_path_name}/Debug" __deps_debug_hashes __deps_current_version)
                # DownloadFile("v8_monolith.lib" "${__base_path}/lib/Debug" "${__deps_os_path_name}/Debug" ${__deps_debug_hashes})
            endif()
        elseif(UNIX)
            message("Checking binaries...")

            GetCDNInfo("${__deps_url_base_path}/${__deps_os_path_name}" __deps_linux_hashes __deps_current_version)

            DownloadFile("libv8_monolith.a" "${__base_path}/lib" "${__deps_os_path_name}" ${__deps_linux_hashes})
        endif()

        GetCDNInfo("${__deps_url_base_path}/include" __deps_headers_hashes __deps_current_version)
        DownloadFile("include.zip" "${__base_path}/include" "include" ${__deps_headers_hashes})
        file(ARCHIVE_EXTRACT INPUT "${__base_path}/include/include.zip" DESTINATION "${__base_path}/include")
        file(REMOVE "${__base_path}/include/include.zip")

        if(__deps_current_version)
            message("V8 deps version: ${__deps_current_version}")
        endif()
    endif()
endfunction()
