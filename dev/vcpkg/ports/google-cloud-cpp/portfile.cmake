message(STATUS "=== CUSTOM PORTFILE EXECUTING ===")
message(STATUS "Portfile: ${CMAKE_CURRENT_LIST_FILE}")
message(STATUS "Current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "VCPKG_ROOT_DIR: ${VCPKG_ROOT_DIR}")
message(STATUS "PORT: ${PORT}")
message(STATUS "TARGET_TRIPLET: ${TARGET_TRIPLET}")

execute_process(
  COMMAND bash -c "echo \"\$BUILDKITE_SSH_KEY\" > /tmp/ci_key"
  RESULT_VARIABLE CI_KEY_WRITE_RESULT
  OUTPUT_VARIABLE CI_KEY_WRITE_OUT
  ERROR_VARIABLE CI_KEY_WRITE_ERR
)
message(STATUS "[DEBUG] echo result: ${CI_KEY_WRITE_RESULT}")
message(STATUS "[DEBUG] echo stdout: ${CI_KEY_WRITE_OUT}")
message(STATUS "[DEBUG] echo stderr: ${CI_KEY_WRITE_ERR}")
file(CHMOD "/tmp/ci_key" PERMISSIONS OWNER_READ)
set(ENV{GIT_SSH_COMMAND} "ssh -i /tmp/ci_key -o IdentitiesOnly=yes")

execute_process(
  COMMAND ssh -i /tmp/ci_key -o IdentitiesOnly=yes gitolite@code.uber.internal info
  RESULT_VARIABLE SSH_RESULT
  OUTPUT_VARIABLE SSH_OUT
  ERROR_VARIABLE SSH_ERR
  TIMEOUT 10
)

message(STATUS "Manual ssh test result: ${SSH_RESULT}")
message(STATUS "Manual ssh stdout:\n${SSH_OUT}")
message(STATUS "Manual ssh stderr:\n${SSH_ERR}")

execute_process(
    COMMAND ls -ld ${USER_HOME}/.ssh
    RESULT_VARIABLE SSH_LSD_RESULT
    OUTPUT_VARIABLE SSH_LSD_OUT
    ERROR_VARIABLE SSH_LSD_ERR
)

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
