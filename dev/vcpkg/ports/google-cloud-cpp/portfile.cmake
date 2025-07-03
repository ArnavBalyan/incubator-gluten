message(STATUS "=== CUSTOM PORTFILE EXECUTING ===")
message(STATUS "Portfile: ${CMAKE_CURRENT_LIST_FILE}")
message(STATUS "Current directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "VCPKG_ROOT_DIR: ${VCPKG_ROOT_DIR}")
message(STATUS "PORT: ${PORT}")
message(STATUS "TARGET_TRIPLET: ${TARGET_TRIPLET}")

execute_process(COMMAND whoami OUTPUT_VARIABLE WHOAMI OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "VCPKG build user: ${WHOAMI}")

# Diagnostic: Print effective home directory
execute_process(COMMAND printenv HOME OUTPUT_VARIABLE USER_HOME OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "HOME directory: ${USER_HOME}")

# Diagnostic: List contents of ~/.ssh
execute_process(
    COMMAND ls -l ${USER_HOME}/.ssh
    RESULT_VARIABLE SSH_LS_RESULT
    OUTPUT_VARIABLE SSH_LS_OUT
    ERROR_VARIABLE SSH_LS_ERR
)
message(STATUS "~/.ssh contents:")
message(STATUS "${SSH_LS_OUT}")
if(NOT "${SSH_LS_ERR}" STREQUAL "")
    message(STATUS "Error listing ~/.ssh: ${SSH_LS_ERR}")
endif()

# Diagnostic: Check GIT_SSH_COMMAND if set
execute_process(COMMAND printenv GIT_SSH_COMMAND OUTPUT_VARIABLE GIT_SSH_COMMAND_VAL OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "GIT_SSH_COMMAND: ${GIT_SSH_COMMAND_VAL}")

# Diagnostic: Show default SSH identity used by git
execute_process(
    COMMAND ssh -v gitolite@code.uber.internal
    RESULT_VARIABLE SSH_RESULT
    OUTPUT_VARIABLE SSH_OUT
    ERROR_VARIABLE SSH_ERR
    TIMEOUT 10
)
message(STATUS "=== SSH direct connection attempt ===")
if(NOT "${SSH_OUT}" STREQUAL "")
    message(STATUS "SSH stdout: ${SSH_OUT}")
endif()
if(NOT "${SSH_ERR}" STREQUAL "")
    message(STATUS "SSH stderr: ${SSH_ERR}")
endif()
message(STATUS "SSH return code: ${SSH_RESULT}")

# Diagnostic: Run git ls-remote manually
execute_process(
    COMMAND git ls-remote gitolite@code.uber.internal:data/google-cloud-cpp
    WORKING_DIRECTORY "${CURRENT_BUILDTREES_DIR}"
    RESULT_VARIABLE LS_REMOTE_RESULT
    OUTPUT_VARIABLE LS_REMOTE_OUT
    ERROR_VARIABLE LS_REMOTE_ERR
)
message(STATUS "=== git ls-remote diagnostics ===")
message(STATUS "ls-remote result: ${LS_REMOTE_RESULT}")
message(STATUS "ls-remote output:\n${LS_REMOTE_OUT}")
message(STATUS "ls-remote error:\n${LS_REMOTE_ERR}")


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
