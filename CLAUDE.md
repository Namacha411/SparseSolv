# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

### Using Conan (Primary build system)
```bash
# Install dependencies and build
conan install . --build=missing
cmake --preset conan-default
cmake --build --preset conan-release

# Test the package
conan create . --build=missing

# Build with options
conan install . --build=missing -o sparse-solv/*:with_python_bindings=True -o sparse-solv/*:with_mkl=True
```

### Development workflow
```bash
# Quick build after making changes
cmake --build --preset conan-release

# Run the test package example
./build/Release/test_package/example
```

### Python bindings
```bash
# Build with Python bindings enabled
conan install . --build=missing -o sparse-solv/*:with_python_bindings=True
cmake --preset conan-default
cmake --build --preset conan-release

# Test Python bindings
python examples/Pybind_example.py
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

**Package Management**
- Primary distribution via Conan packages
- CMake config files for downstream integration
- Header-only template components for performance

### Key Implementation Details

**Matrix Storage**: Uses Eigen's sparse matrix infrastructure with custom CRS optimizations for iterative solvers

**Memory Management**: Template-based design with move semantics and RAII patterns throughout

**Parallelization**: OpenMP threading in matrix-vector operations and specialized multi-color orderings for parallel preconditioning

**Solver Configuration**: Configurable diagonal scaling, convergence monitoring, and divergence detection with best-solution preservation

### Examples and Testing

**VoxelFEM Example** (`examples/VoxelFEM/`)
- Complete electromagnetic finite element analysis using voxel discretization
- Demonstrates matrix assembly and solver usage in practical application
- Requires external mesh data download

**Test Package** (`test_package/`)
- Conan package validation with basic matrix creation and solver instantiation
- Tests both C++ and C API functionality