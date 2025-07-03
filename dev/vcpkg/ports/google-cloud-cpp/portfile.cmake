message(STATUS "=== CUSTOM PORTFILE EXECUTING ===")
message(STATUS "Portfile: ${CMAKE_CURRENT_LIST_FILE}")
message(STATUS "Current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "VCPKG_ROOT_DIR: ${VCPKG_ROOT_DIR}")
message(STATUS "PORT: ${PORT}")
message(STATUS "TARGET_TRIPLET: ${TARGET_TRIPLET}")


execute_process(
    COMMAND git ls-remote gitolite@code.uber.internal:data/google-cloud-cpp
    WORKING_DIRECTORY "${CURRENT_BUILDTREES_DIR}"
    RESULT_VARIABLE LS_REMOTE_RESULT
    OUTPUT_VARIABLE LS_REMOTE_OUT
    ERROR_VARIABLE LS_REMOTE_ERR
)

message(STATUS "ls-remote result: ${LS_REMOTE_RESULT}")
message(STATUS "ls-remote output: ${LS_REMOTE_OUT}")
message(STATUS "ls-remote error: ${LS_REMOTE_ERR}")



vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL gitolite@code.uber.internal:data/google-cloud-cpp
    REF 28d1e7b2de64f8cce12ab98c6033073da044f114
)

vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DBUILD_TESTING=OFF
)

function (google_cloud_cpp_cmake_config_fixup library)
    string(REPLACE "-" "_" library "${library}")
    set(config_path "lib/cmake/google_cloud_cpp_${library}")
    if(NOT IS_DIRECTORY "${CURRENT_PACKAGES_DIR}/${config_path}")
        message(STATUS "Skipping ${library}: ${config_path} not found")
        return()
    endif()
    vcpkg_cmake_config_fixup(
        PACKAGE_NAME "google_cloud_cpp_${library}"
        CONFIG_PATH "${config_path}"
        DO_NOT_DELETE_PARENT_CONFIG_PATH
    )
endfunction ()

vcpkg_cmake_install()

foreach(feature IN ITEMS storage)
    google_cloud_cpp_cmake_config_fixup(${feature})
endforeach()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
