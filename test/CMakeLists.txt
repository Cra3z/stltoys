add_executable(
    test_string
    test_string.cpp
)
add_executable(
    test_vector
    test_vector.cpp
)

foreach(TEST_NAME IN ITEMS string vector)
    gtest_discover_tests(test_${TEST_NAME})

    target_include_directories(
        test_${TEST_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
    )

    target_link_libraries(
        test_${TEST_NAME}
        PRIVATE
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
    )
endforeach()
