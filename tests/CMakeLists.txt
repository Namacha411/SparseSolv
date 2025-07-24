cmake_minimum_required(VERSION 3.15)

# Find required packages
find_package(sparse-solv CONFIG REQUIRED)
find_package(GTest REQUIRED)

# Enable testing
enable_testing()

# Test executables
set(TEST_SOURCES
    test_sparse_mat.cpp
    test_sparse_mat_c.cpp
    test_mat_solvers.cpp
    test_api.cpp
)

foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} 
        sparse-solv::sparse-solv 
        GTest::gtest 
        GTest::gtest_main
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

# Integration test
add_executable(integration_test integration_test.cpp)
target_link_libraries(integration_test 
    sparse-solv::sparse-solv 
    GTest::gtest 
    GTest::gtest_main
)
add_test(NAME integration_test COMMAND integration_test)