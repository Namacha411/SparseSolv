# SparseSolv

[![Conan Package](https://img.shields.io/badge/conan-sparse--solv%2F0.1.0-blue)](https://conan.io/)

High-performance sparse linear algebra library with iterative solvers including:
- Shifted-ICCG (Incomplete Cholesky Conjugate Gradient)
- Multicolor ordering shifted ICCG with ABMC (Algebraic Block Multi-Color)
- Shifted-IC+MRTR (Incomplete Cholesky + Minimum Residual Three-term Recurrence)
- Eisenstat's Symmetric Gauss-Seidel-MRTR

## Overview

SparseSolv is a comprehensive sparse matrix library optimized for electromagnetic finite element analysis and other scientific computing applications. It provides both C++ and Python APIs with support for Intel MKL acceleration and OpenMP parallelization.

## Installation

### Using Conan (Recommended)

#### Basic Installation
```bash
conan install --requires="sparse-solv/0.1.0"
```

#### With Python Bindings
```bash
conan install --requires="sparse-solv/0.1.0" -o sparse-solv/*:with_python_bindings=True
```

#### With Intel MKL Support
```bash
conan install --requires="sparse-solv/0.1.0" -o sparse-solv/*:with_mkl=True
```

#### All Features Enabled
```bash
conan install --requires="sparse-solv/0.1.0" \
    -o sparse-solv/*:with_python_bindings=True \
    -o sparse-solv/*:with_mkl=True \
    -o sparse-solv/*:with_openmp=True
```

### Building from Source

#### Requirements
- CMake 3.15+
- C++17 compatible compiler
- Conan 2.0+

#### Development Setup
```bash
git clone https://github.com/JP-MARs/SparseSolv.git
cd SparseSolv

# Install dependencies
conan install . --build=missing

# Build
cmake --preset conan-default
cmake --build --preset conan-release

# Test the package
conan create . --build=missing

# Build with tests enabled for development
conan install . --build=missing -o sparse-solv/*:build_tests=True
cmake --preset conan-release
cmake --build --preset conan-release
ctest --test-dir build/Release
```

## Usage

### C++ API

```cpp
#include <sparse-solv.h>

int main() {
    // Create sparse matrix
    auto mat = SparseSolv::createSparseMat(1000);
    
    // Add matrix entries
    mat->add(0, 0, 2.0);
    mat->add(0, 1, -1.0);
    // ... add more entries
    
    // Finalize matrix structure
    mat->fix();
    
    // Create solver
    auto solver = SparseSolv::createSolver();
    
    // Configure solver
    solver->setDiagScale(true);
    solver->setSaveBest(true);
    
    // Solve system: A * x = b
    std::vector<double> b(1000, 1.0);  // RHS vector
    std::vector<double> x(1000, 0.0);  // Solution vector
    
    bool converged = solver->solveICCG(1000, 1e-6, 1000, 1.02, 
                                       *mat, b, x);
    
    return converged ? 0 : 1;
}
```

### C API

```c
#include <sparse-solv.h>

int main() {
    // Create matrix and solver
    void* mat = sparse_mat_create(1000);
    void* solver = mat_solvers_create();
    
    // Add matrix entries
    sparse_mat_add(mat, 0, 0, 2.0);
    sparse_mat_add(mat, 0, 1, -1.0);
    
    // Finalize and solve
    sparse_mat_fix(mat);
    
    double* b = malloc(1000 * sizeof(double));
    double* x = malloc(1000 * sizeof(double));
    // ... initialize b and x
    
    bool success = mat_solvers_iccg(solver, 1000, 1e-6, 1000, 1.02, 
                                    mat, b, x);
    
    // Cleanup
    free(b);
    free(x);
    sparse_mat_destroy(mat);
    mat_solvers_destroy(solver);
    
    return success ? 0 : 1;
}
```

### Python API

```python
import SparseSolvPy

# Create sparse matrix
mat = SparseSolvPy.SparseMat(1000)

# Add matrix entries
mat.add(0, 0, 2.0)
mat.add(0, 1, -1.0)
# ... add more entries

# Finalize matrix
mat.fix()

# Create and configure solver
solver = SparseSolvPy.MatSolvers()
solver.setDiagScale(True)
solver.setSaveBest(True)

# Solve system
b = [1.0] * 1000  # RHS vector
x = [0.0] * 1000  # Solution vector

converged = solver.solveICCG(1000, 1e-6, 1000, 1.02, mat, b, x)
print(f"Converged: {converged}")
```

## Testing

SparseSolv includes a comprehensive test suite built with **Google Test (gtest)** covering all major functionality:

### Running Tests

```bash
# Install with tests enabled (includes gtest dependency)
conan install . --build=missing -o sparse-solv/*:build_tests=True

# Configure and build with tests
cmake --preset conan-release
cmake --build --preset conan-release

# Run all tests with CTest (recommended)
cd build/Release && ctest --verbose

# Run individual gtest executables with filtering capabilities
./build/Release/test_sparse_mat --gtest_filter="*Creation*"
./build/Release/test_sparse_mat_c --gtest_filter="*Complex*"
./build/Release/test_mat_solvers --gtest_filter="*ICCG*"
./build/Release/test_api --gtest_filter="*CAPI*"
./build/Release/integration_test --gtest_filter="*Performance*"

# Run all tests with XML output for CI/CD
./build/Release/test_sparse_mat --gtest_output=xml:test_results.xml

# Run basic test executables (all tests)
./build/Release/test_sparse_mat      # SparseMat functionality tests
./build/Release/test_sparse_mat_c    # Complex matrix functionality tests
./build/Release/test_mat_solvers     # Solver algorithm tests
./build/Release/test_api             # C API validation tests
./build/Release/integration_test     # Integration and performance tests
```

### Python Binding Tests

```bash
# Requires Python bindings to be built
python tests/test_python_bindings.py
```

### Test Coverage

Built with **Google Test framework** for robust testing:

- **Unit Tests**: Core matrix operations, solver algorithms, API functionality
- **Integration Tests**: Real-world problem scenarios, performance benchmarks
- **Memory Tests**: Memory leak detection with Valgrind (when available)
- **API Tests**: Both C++ and C API validation with comprehensive fixtures
- **Python Tests**: Python binding functionality and NumPy integration

### Test Features

- **gtest Fixtures**: Organized test classes with setup/teardown
- **Rich Assertions**: `ASSERT_EQ`, `ASSERT_NEAR`, `ASSERT_TRUE`, etc.
- **Test Filtering**: Run specific tests with `--gtest_filter="pattern"`
- **XML Output**: CI/CD integration with `--gtest_output=xml`
- **Detailed Reporting**: Enhanced failure diagnostics and test output

## Package Options

| Option | Default | Description |
|--------|---------|-------------|
| `shared` | `False` | Build shared library |
| `fPIC` | `True` | Position independent code |
| `with_python_bindings` | `False` | Build Python bindings |
| `with_mkl` | `False` | Enable Intel MKL support |
| `with_openmp` | `True` | Enable OpenMP parallelization |
| `build_tests` | `False` | Build comprehensive test suite |

## CMake Integration

If using SparseSolv in your CMake project:

```cmake
find_package(SparseSolv REQUIRED)
target_link_libraries(your_target SparseSolv::sparse-solv)

# For Python bindings component
find_package(SparseSolv REQUIRED COMPONENTS python-bindings)
target_link_libraries(your_target SparseSolv::python-bindings)
```

## Algorithms

### ICCG (Incomplete Cholesky Conjugate Gradient)
- Accelerated ICCG with shift parameter
- Diagonal scaling support
- Convergence monitoring with divergence detection

### ABMC-ICCG (Algebraic Block Multi-Color ICCG)
- Parallel ICCG using algebraic multi-color ordering
- Block-based parallelization for improved cache efficiency
- Suitable for large-scale problems on multi-core systems

### IC-MRTR (Incomplete Cholesky + MRTR)
- Three-term recurrence relation solver
- Reduced memory requirements compared to CG methods
- Stable convergence properties

### SGS-MRTR (Symmetric Gauss-Seidel MRTR)
- Eisenstat's symmetric Gauss-Seidel preconditioner
- Combined with MRTR for improved convergence
- Effective for poorly conditioned systems

## Examples

### VoxelFEM Example
A complete electromagnetic finite element analysis example using voxel elements:

```bash
cd examples/VoxelFEM
# Download mesh data from:
# https://u.muroran-it.ac.jp/it-elec-lab/open_data/voxel_data/
make
./VoxelFEM
```

### Python Example
```bash
python examples/Pybind_example.py
```

## Performance Considerations

### Compiler Optimizations
- Intel compilers: Automatic IPO and MKL linking
- GCC/Clang: `-O3` optimization with OpenMP support
- Platform-specific SIMD optimizations through Eigen

### Memory Usage
- Compressed row storage (CRS) format
- Efficient incomplete factorization storage
- Configurable block sizes for cache optimization

### Parallelization
- OpenMP parallel matrix-vector operations
- Multi-color ordering for parallel preconditioning
- Thread-safe solver interfaces

## Contributors

- **Takahiro Sato** (Muroran Institute of Technology, JAPAN)
- **Shingo Hiruma** (Kyoto University, JAPAN) - ABMC ordering implementation
- **Kengo Sugahara** (Kindai University, JAPAN)
- **Tomonori Tsuburaya** (Fukuoka University, JAPAN) - MRTR methods

## License

MPL-2.0 License - see [LICENSE](LICENSE) file for details.

## Citation

If you use SparseSolv in your research, please cite:

```bibtex
@software{sparsesolv2024,
  title={SparseSolv: High-Performance Sparse Linear Algebra Library},
  author={Sato, Takahiro and Hiruma, Shingo and Sugahara, Kengo and Tsuburaya, Tomonori},
  year={2024},
  url={https://github.com/JP-MARs/SparseSolv}
}
```

---

## 本ライブラリの説明 (Japanese Description)

本ライブラリは、日本の磁界系数値解析の研究者による疎行列ソルバのライブラリです。電磁界有限要素法のソルバとして広く使われている加速係数付きICCG法の線形ソルバです。また、MRTR法も実装されています。

本ライブラリはConanパッケージとして提供され、C++とPython APIの両方をサポートしています。Intel MKLサポートとOpenMP並列化により、高性能な計算が可能です。

### 主な特徴
- 加速係数付きICCG法
- マルチカラー順序付きICCG (ABMC)
- IC+MRTR法およびSGS-MRTR法
- Python bindings (pybind11)
- Intel MKL対応
- OpenMP並列化
- Conanパッケージマネージャー対応
