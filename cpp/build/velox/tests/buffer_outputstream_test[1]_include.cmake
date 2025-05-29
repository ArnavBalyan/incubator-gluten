if(EXISTS "/home/user/gluten/cpp/build/velox/tests/buffer_outputstream_test")
  if(NOT EXISTS "/home/user/gluten/cpp/build/velox/tests/buffer_outputstream_test[1]_tests.cmake" OR
     NOT "/home/user/gluten/cpp/build/velox/tests/buffer_outputstream_test[1]_tests.cmake" IS_NEWER_THAN "/home/user/gluten/cpp/build/velox/tests/buffer_outputstream_test" OR
     NOT "/home/user/gluten/cpp/build/velox/tests/buffer_outputstream_test[1]_tests.cmake" IS_NEWER_THAN "${CMAKE_CURRENT_LIST_FILE}")
    include("/home/user/gluten/dev/vcpkg/.vcpkg/downloads/tools/cmake-3.28.3-linux/cmake-3.28.3-linux-x86_64/share/cmake-3.28/Modules/GoogleTestAddTests.cmake")
    gtest_discover_tests_impl(
      TEST_EXECUTABLE [==[/home/user/gluten/cpp/build/velox/tests/buffer_outputstream_test]==]
      TEST_EXECUTOR [==[]==]
      TEST_WORKING_DIR [==[/home/user/gluten/cpp/build/velox/tests]==]
      TEST_EXTRA_ARGS [==[]==]
      TEST_PROPERTIES [==[]==]
      TEST_PREFIX [==[]==]
      TEST_SUFFIX [==[]==]
      TEST_FILTER [==[]==]
      NO_PRETTY_TYPES [==[FALSE]==]
      NO_PRETTY_VALUES [==[FALSE]==]
      TEST_LIST [==[buffer_outputstream_test_TESTS]==]
      CTEST_FILE [==[/home/user/gluten/cpp/build/velox/tests/buffer_outputstream_test[1]_tests.cmake]==]
      TEST_DISCOVERY_TIMEOUT [==[5]==]
      TEST_XML_OUTPUT_DIR [==[]==]
    )
  endif()
  include("/home/user/gluten/cpp/build/velox/tests/buffer_outputstream_test[1]_tests.cmake")
else()
  add_test(buffer_outputstream_test_NOT_BUILT buffer_outputstream_test_NOT_BUILT)
endif()
