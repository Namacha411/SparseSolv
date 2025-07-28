cmake_minimum_required(VERSION 3.15)
project(SparseSolv VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options (set by conanfile.py through CMakeToolchain)
option(SPARSE_SOLV_WITH_PYTHON_BINDINGS "Build Python bindings" OFF)
option(SPARSE_SOLV_WITH_MKL "Enable Intel MKL support" OFF)
option(SPARSE_SOLV_WITH_OPENMP "Enable OpenMP support" ON)

# Find dependencies
find_package(Eigen3 REQUIRED)

# OpenMP support
if(SPARSE_SOLV_WITH_OPENMP)
    find_package(OpenMP REQUIRED)
endif()

# Python bindings support
if(SPARSE_SOLV_WITH_PYTHON_BINDINGS)
    find_package(pybind11 REQUIRED)
endif()

# Collect all source files
set(SPARSESOLV_SOURCES
    src/SparseMat.cpp
    src/SparseMatC.cpp
    src/SparseMatOperators.cpp
    src/MatSolvers_Base.cpp
    src/MatSolvers_ICCG.cpp
    src/MatSolvers_ICCOCG.cpp
    src/MatSolvers_ICMRTR.cpp
    src/MatSolvers_SGSMRTR.cpp
    src/MatSolvers_Eigen.cpp
    src/MatSolvers_MKL.cpp
    src/MatSolvers_ABMCICCG.cpp
    src/sparse-solv-api.cpp
)

# Create main library
add_library(sparse-solv ${SPARSESOLV_SOURCES})

# Set target properties
set_target_properties(sparse-solv PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "include/sparse-solv.h"
)

# Include directories
target_include_directories(sparse-solv 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(sparse-solv PUBLIC Eigen3::Eigen)

# Compiler-specific optimizations and MKL support
if(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    if(CMAKE_CXX_COMPILER MATCHES "icpc")
        # Intel Classical Compiler
        target_compile_options(sparse-solv PRIVATE -O3 -ipo)
        if(SPARSE_SOLV_WITH_OPENMP)
            target_compile_options(sparse-solv PRIVATE -qopenmp)
        endif()
        if(SPARSE_SOLV_WITH_MKL)
            target_link_libraries(sparse-solv PRIVATE -qmkl)
        endif()
    elseif(CMAKE_CXX_COMPILER MATCHES "icpx")
        # Intel OneAPI Compiler
        target_compile_options(sparse-solv PRIVATE -O3)
        if(SPARSE_SOLV_WITH_OPENMP)
            target_compile_options(sparse-solv PRIVATE -qopenmp)
        endif()
        if(SPARSE_SOLV_WITH_MKL)
            target_link_libraries(sparse-solv PRIVATE -qmkl -ipo)
        endif()
    endif()
else()
    # GCC/Clang
    target_compile_options(sparse-solv PRIVATE -O3 -DNDEBUG)
    if(SPARSE_SOLV_WITH_OPENMP)
        target_link_libraries(sparse-solv PUBLIC OpenMP::OpenMP_CXX)
    endif()
endif()

# Python bindings (optional)
if(SPARSE_SOLV_WITH_PYTHON_BINDINGS)
    message(STATUS "Building Python bindings")
    
    set(PYTHON_BINDING_SOURCES
        src/python/pybind_SparseMatPy.cpp
        src/python/pybind_MatSolversPy.cpp
    )
    
    pybind11_add_module(SparseSolvPy ${PYTHON_BINDING_SOURCES})
    
    # Link against the main library
    target_link_libraries(SparseSolvPy PRIVATE sparse-solv)
    
    # Set target properties
    target_compile_definitions(SparseSolvPy PRIVATE VERSION_INFO="${PROJECT_VERSION}")
    
    # Compiler-specific flags for Python bindings
    if(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
        if(CMAKE_CXX_COMPILER MATCHES "icpc")
            target_compile_options(SparseSolvPy PRIVATE -fPIC -O3 -ipo)
            if(SPARSE_SOLV_WITH_OPENMP)
                target_compile_options(SparseSolvPy PRIVATE -qopenmp)
            endif()
            if(SPARSE_SOLV_WITH_MKL)
                target_link_libraries(SparseSolvPy PRIVATE -qmkl)
            endif()
        elseif(CMAKE_CXX_COMPILER MATCHES "icpx")
            target_compile_options(SparseSolvPy PRIVATE -fPIC -O3)
            if(SPARSE_SOLV_WITH_OPENMP)
                target_compile_options(SparseSolvPy PRIVATE -qopenmp)
            endif()
            if(SPARSE_SOLV_WITH_MKL)
                target_link_libraries(SparseSolvPy PRIVATE -qmkl -ipo)
            endif()
        endif()
    else()
        target_compile_options(SparseSolvPy PRIVATE -fPIC -O3)
        if(SPARSE_SOLV_WITH_OPENMP)
            target_link_libraries(SparseSolvPy PRIVATE OpenMP::OpenMP_CXX)
        endif()
    endif()
    
    # Install Python module
    install(TARGETS SparseSolvPy DESTINATION .)
endif()

# Testing (optional)
option(SPARSE_SOLV_BUILD_TESTS "Build tests" OFF)

if(SPARSE_SOLV_BUILD_TESTS)
    enable_testing()
    
    # Find GTest for testing
    find_package(GTest REQUIRED)
    
    # Test executables
    set(TEST_SOURCES
        tests/test_sparse_mat.cpp
        tests/test_sparse_mat_c.cpp
        tests/test_mat_solvers.cpp
        tests/test_api.cpp
        tests/integration_test.cpp
    )
    
    foreach(test_source ${TEST_SOURCES})
        get_filename_component(test_name ${test_source} NAME_WE)
        add_executable(${test_name} ${test_source})
        target_link_libraries(${test_name} 
            sparse-solv 
            GTest::gtest 
            GTest::gtest_main
        )
        target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
    
    message(STATUS "Building tests enabled")
endif()

# Installation
install(TARGETS sparse-solv
    EXPORT SparseSolvTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

# Export additional headers
install(DIRECTORY src/ 
    DESTINATION include/sparse-solv
    FILES_MATCHING 
    PATTERN "*.hpp"
    PATTERN "*.h"
)

# Install CMake config files
install(EXPORT SparseSolvTargets
    FILE SparseSolvTargets.cmake
    NAMESPACE SparseSolv::
    DESTINATION lib/cmake/SparseSolv
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SparseSolvConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SparseSolvConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SparseSolvConfig.cmake"
    INSTALL_DESTINATION lib/cmake/SparseSolv
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/SparseSolvConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/SparseSolvConfigVersion.cmake"
    DESTINATION lib/cmake/SparseSolv
)