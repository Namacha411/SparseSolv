# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### Using Conan (Primary build system)
```bash
# Install dependencies and build
conan install . --build=missing
cmake --preset conan-release
cmake --build --preset conan-release

# Test the package
conan create . --build=missing

# Build with options
conan install . --build=missing -o sparse-solv/*:with_python_bindings=True -o sparse-solv/*:with_mkl=True
```

### Development workflow with tests
```bash
# Build with comprehensive test suite (now using GoogleTest)
conan install . --build=missing -o sparse-solv/*:build_tests=True
cmake --preset conan-release
cmake --build --preset conan-release

# Run all tests via CTest (recommended)
cd build/Release && ctest --verbose

# Run individual gtest executables with filtering capabilities
./build/Release/test_sparse_mat --gtest_filter="*Creation*"
./build/Release/test_sparse_mat_c --gtest_filter="*Complex*"
./build/Release/test_mat_solvers --gtest_filter="*ICCG*"
./build/Release/test_api --gtest_filter="*CAPI*"
./build/Release/integration_test --gtest_filter="*Performance*"

# Run all tests with XML output for CI
./build/Release/test_sparse_mat --gtest_output=xml:test_results.xml

# Quick build after making changes
cmake --build --preset conan-release

# Run the gtest-based test package example
./test_package/build/gcc-12-x86_64-gnu17-release/example
```

### Python bindings and tests
```bash
# Build with Python bindings enabled
conan install . --build=missing -o sparse-solv/*:with_python_bindings=True
cmake --preset conan-release
cmake --build --preset conan-release

# Test Python bindings using uv
uv run python examples/Pybind_example.py
uv run python tests/test_python_bindings.py

# Alternative: using direct python (if module path configured)
# python examples/Pybind_example.py
# python tests/test_python_bindings.py
```

### Testing Commands
```bash
# Memory leak testing (requires Valgrind)
valgrind --leak-check=full ./build/Release/test_sparse_mat

# Performance testing
./build/Release/integration_test

# Test with different build configurations
conan install . --build=missing -s build_type=Debug -o sparse-solv/*:build_tests=True
cmake --preset conan-debug
cmake --build --preset conan-debug
```

## Project Architecture

### Core Components

**SparseMat** (`src/SparseMat.hpp`, `src/SparseMat.cpp`)
- Real-valued sparse matrix implementation using compressed row storage (CRS)
- Template-based architecture with `SparseMatBaseD` (double precision)
- Key methods: `add()`, `fix()`, matrix-vector multiplication operators

**SparseMatC** (`src/SparseMatC.hpp`, `src/SparseMatC.cpp`) 
- Complex-valued sparse matrix counterpart using `SparseMatBaseC`
- Supports complex arithmetic and operations

**MatSolvers** (`src/MatSolvers.hpp`)
- Collection of iterative linear solvers optimized for electromagnetic FEM
- Core algorithms:
  - **ICCG**: Incomplete Cholesky Conjugate Gradient with acceleration
  - **ABMC-ICCG**: Algebraic Block Multi-Color parallel ICCG (by Shingo Hiruma)
  - **IC-MRTR**: Incomplete Cholesky + Minimum Residual Three-term Recurrence (by Tomonori Tsuburaya)
  - **SGS-MRTR**: Symmetric Gauss-Seidel MRTR (Eisenstat variant)

### Language Bindings

**C++ API** (`include/sparse-solv.h`)
- Native C++ interface with smart pointers and RAII
- Convenience factory functions in `SparseSolv` namespace

**C API** (`src/sparse-solv-api.cpp`)
- Exports C-compatible functions for language bindings
- Handles void* opaque pointers for cross-language compatibility

**Python Bindings** (`src/python/`)
- `pybind_SparseMatPy.cpp`: Exposes SparseMat and SparseMatC classes
- `pybind_MatSolversPy.cpp`: Exposes MatSolvers with all algorithm variants
- Module name: `SparseSolvPy`

### Build System Features

**Compiler Optimizations**
- Intel compilers: Automatic IPO (`-ipo`) and MKL linking (`-qmkl`)
- GCC/Clang: `-O3` optimization with OpenMP support
- Architecture-specific optimizations through Eigen

**Optional Dependencies**
- `SPARSE_SOLV_WITH_MKL`: Intel MKL acceleration
- `SPARSE_SOLV_WITH_OPENMP`: Parallel matrix operations
- `SPARSE_SOLV_WITH_PYTHON_BINDINGS`: pybind11 Python interface
- `SPARSE_SOLV_BUILD_TESTS`: Comprehensive test suite

**Package Management**
- Primary distribution via Conan packages
- CMake config files for downstream integration
- Header-only template components for performance
- Automated testing with CTest integration

### Key Implementation Details

**Matrix Storage**: Uses Eigen's sparse matrix infrastructure with custom CRS optimizations for iterative solvers

**Memory Management**: Template-based design with move semantics and RAII patterns throughout

**Parallelization**: OpenMP threading in matrix-vector operations and specialized multi-color orderings for parallel preconditioning

**Solver Configuration**: Configurable diagonal scaling, convergence monitoring, and divergence detection with best-solution preservation

### Testing Infrastructure

**Comprehensive Test Suite** (`tests/`) - **Built with Google Test (gtest)**
- `test_sparse_mat.cpp`: SparseMat functionality tests with `SparseMatTest` fixture
  - Creation, operations, large matrices, empty operations
- `test_sparse_mat_c.cpp`: Complex matrix tests with `SparseMatCTest` fixture
  - Complex operations, Hermitian matrices, mixed real/complex
- `test_mat_solvers.cpp`: All solver algorithm tests with `MatSolversTest` fixture
  - ICCG, IC-MRTR, SGS-MRTR solvers with helper methods for vector comparisons
- `test_api.cpp`: C API validation with `CAPITest` fixture
  - Memory management, function validation, error handling
- `integration_test.cpp`: Real-world scenarios with `IntegrationTest` fixture
  - FEM-like systems, performance benchmarks, solver comparisons
- `test_python_bindings.py`: Python API tests with NumPy integration

**Test Package** (`test_package/`) - **Google Test enabled**
- Conan package validation with gtest-based tests
- `TestPackageTest` fixture for organized validation
- Tests both C++ and C API functionality
- Validates package installation and linking

**Examples and Testing**

**VoxelFEM Example** (`examples/VoxelFEM/`)
- Complete electromagnetic finite element analysis using voxel discretization
- Demonstrates matrix assembly and solver usage in practical application
- Requires external mesh data download

**Python Example** (`examples/Pybind_example.py`)
- Demonstrates modern Python API usage with `uv run`
- Shows integration with NumPy arrays
- Tests both real and complex sparse matrices
- Examples of ICCG, IC-MRTR, and SGS-MRTR solvers

**Python API Features**
- Modern function-based solver interface returning (solution, converged) tuples
- Support for both real (`SparseMat`) and complex (`SparseMatC`) sparse matrices
- Integration with Python lists and NumPy arrays
- Automatic module path detection in examples and tests

### Testing and Quality Assurance

**Automated Testing Commands**
When developing or modifying code, ALWAYS run tests:
```bash
# Essential test workflow with gtest
conan install . --build=missing -o sparse-solv/*:build_tests=True
cmake --preset conan-release
cmake --build --preset conan-release
cd build/Release && ctest --verbose
```

**Google Test Features**
- **Test Fixtures**: Organized test classes with setup/teardown methods
- **Rich Assertions**: `ASSERT_EQ`, `ASSERT_NEAR`, `ASSERT_TRUE`, `EXPECT_*` variants
- **Test Filtering**: Run specific tests with `--gtest_filter="pattern"`
- **XML Output**: CI/CD integration with `--gtest_output=xml:results.xml`
- **Parameterized Tests**: Data-driven testing capabilities
- **Death Tests**: Testing error conditions and crashes

**Test Coverage Guidelines**
- Unit tests cover all core matrix operations using gtest fixtures
- Integration tests verify solver performance with detailed benchmarking
- Memory leak detection with Valgrind when available
- Python API tests ensure binding compatibility
- API tests validate both C++ and C interfaces with comprehensive fixtures

**Performance Validation**
- `IntegrationTest` fixture includes performance benchmarks
- Memory usage monitoring for large-scale problems
- Convergence rate testing across different problem types
- Solver comparison tests with timing measurements

**Development Best Practices**
When making changes to SparseSolv code:
1. ALWAYS run the test suite after modifications
2. Use the gtest commands above to build with tests enabled
3. Verify both unit tests and integration tests pass
4. Use gtest filtering to run specific test categories during development
5. Check for memory leaks in critical code paths
6. Utilize gtest's detailed error reporting for debugging
7. Test Python bindings with `uv run python tests/test_python_bindings.py`

**Troubleshooting Python Bindings**
If Python examples fail to run:
1. Ensure the project is built with Python bindings enabled:
   ```bash
   conan install . --build=missing -o sparse-solv/*:with_python_bindings=True
   cmake --preset conan-release
   cmake --build --preset conan-release
   ```
2. Use `uv` to manage Python dependencies: `uv add numpy`
3. The module is automatically located via sys.path in examples and tests
4. Check that `SparseSolvPy.cpython-*.so` exists in `build/Release/`

### Google Test Migration Notes

**Migration Summary**: The test suite has been successfully migrated from custom test macros to Google Test framework:

**Key Changes**:
- **C++ Standard**: Upgraded from C++14 to C++17 to meet gtest requirements
- **Dependencies**: Added `gtest/1.14.0` via Conan
- **Test Structure**: Converted all tests to use gtest fixtures and assertions
- **Build System**: Updated CMakeLists.txt files to link with gtest libraries
- **API Enhancements**: Added missing `createSparseMatC()` function

**Test Organization**:
- Each test file now uses a dedicated test fixture class (e.g., `SparseMatTest`, `MatSolversTest`)
- Helper methods for vector comparisons and complex number assertions
- Proper setup/teardown methods for test initialization

**Benefits of Migration**:
- Better test organization and maintainability
- Rich assertion macros with detailed error messages
- Test filtering and selection capabilities
- XML output for CI/CD integration
- Parameterized testing support for future enhancements