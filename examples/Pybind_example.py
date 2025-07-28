"""
SparseSolv Python example using the modern API
Execute with: uv run python examples/Pybind_example.py
"""

import numpy as np
import sys
import os

# Add the build directory to Python path to find the compiled module
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "build", "Release"))

try:
    import SparseSolvPy
except ImportError as e:
    print(f"Error importing SparseSolvPy: {e}")
    print("Make sure the project is built with Python bindings enabled:")
    print(
        "  conan install . --build=missing -o sparse-solv/*:with_python_bindings=True"
    )
    print("  cmake --preset conan-release")
    print("  cmake --build --preset conan-release")
    sys.exit(1)


def main():
    print("SparseSolv Python Example")
    print("=" * 40)

    # Create a 5x5 test matrix
    print("Creating 5x5 sparse matrix...")
    temp_mat = [
        [10.0, 0.0, -0.1, 1.5, 0.01],
        [0.0, 15.6, 0.0, 0.0, 0.0],
        [-0.1, 0.0, 30.5, 0.0, 1.25],
        [1.5, 0.0, 0.0, 24.55, 0.0],
        [0.01, 0.0, 1.25, 0.0, 19.8],
    ]

    mat = SparseSolvPy.SparseMat(5)
    for i in range(5):
        for j in range(5):
            fval = abs(temp_mat[i][j])
            if fval > 1.0e-9:
                mat.add(i, j, temp_mat[i][j])
    mat.fix(False)
    print(f"Matrix is fixed: {mat.isFixed()}")
    mat.printMat("example_matrix.csv")
    print("Matrix saved to example_matrix.csv")

    # Test complex matrix
    print("\nCreating complex sparse matrix...")
    mat2 = SparseSolvPy.SparseMatC(3)
    mat2.add(0, 0, 2.0 + 1.0j)
    mat2.add(1, 1, 3.0 - 0.5j)
    mat2.add(2, 2, 1.5 + 2.0j)
    mat2.fix(False)
    mat2.printMat("example_complex_matrix.csv")
    print("Complex matrix saved to example_complex_matrix.csv")

    # Create a simple 3x3 tridiagonal matrix for testing solvers
    print("\nCreating 3x3 tridiagonal matrix...")
    mat3 = SparseSolvPy.SparseMat(3)
    mat3.add(0, 0, 2.0)
    mat3.add(0, 1, -1.0)
    mat3.add(1, 0, -1.0)
    mat3.add(1, 1, 2.0)
    mat3.add(1, 2, -1.0)
    mat3.add(2, 1, -1.0)
    mat3.add(2, 2, 2.0)
    mat3.fix(False)
    mat3.printMat("example_tridiag_matrix.csv")
    print("Tridiagonal matrix saved to example_tridiag_matrix.csv")

    # Solve linear system
    print("\nSolving linear system Ax = b...")
    solver = SparseSolvPy.MatSolvers()
    solver.setDiagScale(True)
    solver.setSaveLog(True)

    # Right-hand side vector
    rhs = [1.0, 0.5, 0.0, 0.0, 0.0]
    print(f"RHS vector: {rhs}")

    # Solve using SGSMRTR method
    try:
        solution, converged = solver.solveSGSMRTR(
            mat, rhs, conv_cri=1.0e-6, max_ite=1000
        )
        print(f"Converged: {converged}")
        print(f"Solution: {solution[:5]}")  # Show first 5 elements

        # Get residual log
        residual_log = solver.getResidualLog()
        print(f"Iterations: {len(residual_log)}")
        if residual_log:
            print(f"Final residual: {residual_log[-1]:.2e}")
    except Exception as e:
        print(f"Solver failed: {e}")

    # Test ICCG solver
    print("\nTesting ICCG solver...")
    try:
        solution, converged = solver.solveICCG(
            mat3, [3.0, 3.0, 3.0], conv_cri=1.0e-8, max_ite=100
        )
        print(f"ICCG Converged: {converged}")
        print(f"ICCG Solution: {solution}")
    except Exception as e:
        print(f"ICCG solver failed: {e}")

    print("\nExample completed successfully!")


if __name__ == "__main__":
    main()
