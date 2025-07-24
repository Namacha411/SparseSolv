cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(SparseSolv CONFIG REQUIRED)
find_package(GTest REQUIRED)

add_executable(example src/example.cpp)
target_link_libraries(example 
    SparseSolv::sparse-solv 
    GTest::gtest 
    GTest::gtest_main
)
