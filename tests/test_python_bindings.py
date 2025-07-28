"""
Comprehensive tests for SparseSolv Python bindings
Execute with: uv run python tests/test_python_bindings.py
"""

import sys
import os
import unittest
import numpy as np

# Add the build directory to Python path to find the compiled module
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "build", "Release"))

# Try to import the SparseSolvPy module
try:
    import SparseSolvPy

    HAS_PYTHON_BINDINGS = True
except ImportError as e:
    HAS_PYTHON_BINDINGS = False
    print(f"WARNING: SparseSolvPy module not found: {e}")
    print("Python binding tests will be skipped.")
    print("Make sure the project is built with Python bindings enabled:")
    print(
        "  conan install . --build=missing -o sparse-solv/*:with_python_bindings=True"
    )
    print("  cmake --preset conan-release")
    print("  cmake --build --preset conan-release")


@unittest.skipUnless(HAS_PYTHON_BINDINGS, "Python bindings not available")
class TestSparseMat(unittest.TestCase):
    """Test SparseMat Python bindings"""

    def test_creation(self):
        """Test SparseMat creation"""
        mat = SparseSolvPy.SparseMat(5)
        self.assertIsNotNone(mat)

    def test_basic_operations(self):
        """Test basic SparseMat operations"""
        mat = SparseSolvPy.SparseMat(3)

        # Add elements
        mat.add(0, 0, 2.0)
        mat.add(0, 1, -1.0)
        mat.add(1, 0, -1.0)
        mat.add(1, 1, 2.0)
        mat.add(1, 2, -1.0)
        mat.add(2, 1, -1.0)
        mat.add(2, 2, 2.0)

        # Check that matrix is not fixed yet
        self.assertFalse(mat.isFixed())

        # Fix the matrix
        mat.fix(False)
        self.assertTrue(mat.isFixed())

    def test_matrix_vector_multiplication(self):
        """Test matrix-vector multiplication - placeholder test"""
        # Create identity matrix
        mat = SparseSolvPy.SparseMat(3)
        mat.add(0, 0, 1.0)
        mat.add(1, 1, 1.0)
        mat.add(2, 2, 1.0)
        mat.fix(False)

        # Note: multiply_vector method not implemented yet
        # This test will be updated when matrix-vector operations are added
        self.assertTrue(mat.isFixed())

    def test_tridiagonal_system(self):
        """Test with tridiagonal matrix"""
        # Create tridiagonal matrix
        mat = SparseSolvPy.SparseMat(3)
        mat.add(0, 0, 2.0)
        mat.add(0, 1, -1.0)
        mat.add(1, 0, -1.0)
        mat.add(1, 1, 2.0)
        mat.add(1, 2, -1.0)
        mat.add(2, 1, -1.0)
        mat.add(2, 2, 2.0)
        mat.fix(False)

        # Test that the matrix is properly constructed
        # Note: multiply_vector method not implemented yet
        self.assertTrue(mat.isFixed())

    def test_scalar_operations(self):
        """Test scalar multiplication - placeholder test"""
        mat = SparseSolvPy.SparseMat(2)
        mat.add(0, 0, 2.0)
        mat.add(1, 1, 3.0)
        mat.fix(False)

        # Note: multiply_scalar method not implemented yet
        # This test will be updated when scalar operations are added
        self.assertTrue(mat.isFixed())


@unittest.skipUnless(HAS_PYTHON_BINDINGS, "Python bindings not available")
class TestSparseMatC(unittest.TestCase):
    """Test SparseMatC Python bindings"""

    def test_creation(self):
        """Test SparseMatC creation"""
        matc = SparseSolvPy.SparseMatC(5)
        self.assertIsNotNone(matc)

    def test_complex_operations(self):
        """Test complex matrix operations"""
        matc = SparseSolvPy.SparseMatC(2)

        # Add complex elements
        matc.add(0, 0, 2.0 + 1.0j)
        matc.add(1, 1, 3.0 - 1.0j)
        matc.fix(False)

        self.assertTrue(matc.isFixed())

    def test_complex_identity(self):
        """Test complex identity matrix"""
        matc = SparseSolvPy.SparseMatC(3)
        matc.add(0, 0, 1.0 + 0.0j)
        matc.add(1, 1, 1.0 + 0.0j)
        matc.add(2, 2, 1.0 + 0.0j)
        matc.fix(False)

        # Note: multiply_vector method not implemented yet for complex matrices
        # This test will be updated when matrix-vector operations are added
        self.assertTrue(matc.isFixed())

    def test_purely_imaginary(self):
        """Test purely imaginary matrix elements"""
        matc = SparseSolvPy.SparseMatC(2)
        matc.add(0, 0, 2.0j)
        matc.add(0, 1, -1.0j)
        matc.add(1, 0, 1.0j)
        matc.add(1, 1, 3.0j)
        matc.fix(False)

        # Note: multiply_vector method not implemented yet for complex matrices
        # This test will be updated when matrix-vector operations are added
        self.assertTrue(matc.isFixed())


@unittest.skipUnless(HAS_PYTHON_BINDINGS, "Python bindings not available")
class TestMatSolvers(unittest.TestCase):
    """Test MatSolvers Python bindings"""

    def test_creation(self):
        """Test MatSolvers creation"""
        solver = SparseSolvPy.MatSolvers()
        self.assertIsNotNone(solver)

    def test_configuration(self):
        """Test solver configuration"""
        solver = SparseSolvPy.MatSolvers()

        # Test configuration methods
        solver.setDiagScale(True)
        solver.setSaveBest(True)
        solver.setSaveLog(True)
        solver.setDivergeType(1)
        solver.setBadDivVal(10.0)
        solver.setBadDivCount(5)

    def test_iccg_diagonal_system(self):
        """Test ICCG with diagonal system"""
        # Create diagonal matrix
        mat = SparseSolvPy.SparseMat(3)
        mat.add(0, 0, 2.0)
        mat.add(1, 1, 3.0)
        mat.add(2, 2, 4.0)
        mat.fix(False)

        # Right-hand side
        rhs = [4.0, 6.0, 8.0]

        solver = SparseSolvPy.MatSolvers()
        solver.setDiagScale(True)

        # Solve
        solution, converged = solver.solveICCG(
            mat, rhs, conv_cri=1e-10, max_ite=100, accela=1.0
        )

        self.assertTrue(converged)

        # Expected solution: [2, 2, 2]
        self.assertAlmostEqual(solution[0], 2.0, places=6)
        self.assertAlmostEqual(solution[1], 2.0, places=6)
        self.assertAlmostEqual(solution[2], 2.0, places=6)

    def test_iccg_tridiagonal_system(self):
        """Test ICCG with tridiagonal system"""
        # Create tridiagonal matrix
        mat = SparseSolvPy.SparseMat(3)
        mat.add(0, 0, 2.0)
        mat.add(0, 1, -1.0)
        mat.add(1, 0, -1.0)
        mat.add(1, 1, 2.0)
        mat.add(1, 2, -1.0)
        mat.add(2, 1, -1.0)
        mat.add(2, 2, 2.0)
        mat.fix(False)

        # Right-hand side: [1, 0, 1] (solution should be [1, 1, 1])
        rhs = [1.0, 0.0, 1.0]

        solver = SparseSolvPy.MatSolvers()
        solution, converged = solver.solveICCG(
            mat, rhs, conv_cri=1e-10, max_ite=100, accela=1.0
        )

        self.assertTrue(converged)

        # Expected solution: [1, 1, 1]
        self.assertAlmostEqual(solution[0], 1.0, places=5)
        self.assertAlmostEqual(solution[1], 1.0, places=5)
        self.assertAlmostEqual(solution[2], 1.0, places=5)

    def test_abmc_iccg_tridiagonal_system(self):
        """Test ABMC ICCG with tridiagonal system"""
        # Create tridiagonal matrix
        mat = SparseSolvPy.SparseMat(3)
        mat.add(0, 0, 2.0)
        mat.add(0, 1, -1.0)
        mat.add(1, 0, -1.0)
        mat.add(1, 1, 2.0)
        mat.add(1, 2, -1.0)
        mat.add(2, 1, -1.0)
        mat.add(2, 2, 2.0)
        mat.fix(False)

        # Right-hand side: [1, 0, 1] (solution should be [1, 1, 1])
        rhs = [1.0, 0.0, 1.0]

        solver = SparseSolvPy.MatSolvers()
        solution, converged = solver.solveICCGwithABMC(
            mat, rhs, conv_cri=1e-10, max_ite=100, accela=1.0, num_blocks=250, num_colors=4
        )

        self.assertTrue(converged)

        # Expected solution: [1, 1, 1]
        self.assertAlmostEqual(solution[0], 1.0, places=5)
        self.assertAlmostEqual(solution[1], 1.0, places=5)
        self.assertAlmostEqual(solution[2], 1.0, places=5)

    def test_complex_iccg_solver(self):
        """Test complex ICCG solver"""
        # Create complex matrix
        matc = SparseSolvPy.SparseMatC(2)
        matc.add(0, 0, 2.0 + 0.0j)
        matc.add(1, 1, 3.0 + 0.0j)
        matc.fix(False)

        # Right-hand side
        rhs = [4.0 + 2.0j, 6.0 - 3.0j]

        solver = SparseSolvPy.MatSolvers()
        solution, converged = solver.solveComplexICCG(
            matc, rhs, conv_cri=1e-10, max_ite=100, accela=1.0
        )

        self.assertTrue(converged)

        # Expected solution: [(4+2j)/2, (6-3j)/3] = [2+j, 2-j]
        self.assertAlmostEqual(solution[0], 2.0 + 1.0j, places=6)
        self.assertAlmostEqual(solution[1], 2.0 - 1.0j, places=6)

    def test_residual_logging(self):
        """Test residual logging functionality"""
        # Simple diagonal system
        mat = SparseSolvPy.SparseMat(2)
        mat.add(0, 0, 1.0)
        mat.add(1, 1, 1.0)
        mat.fix(False)

        rhs = [1.0, 2.0]

        solver = SparseSolvPy.MatSolvers()
        solver.setSaveLog(True)

        solution, converged = solver.solveICCG(
            mat, rhs, conv_cri=1e-10, max_ite=10, accela=1.0
        )

        self.assertTrue(converged)

        # Get residual log
        residual_log = solver.getResidualLog()
        self.assertGreater(len(residual_log), 0)

    def test_ic_mrtr_solver(self):
        """Test IC-MRTR solver"""
        # Simple system
        mat = SparseSolvPy.SparseMat(3)
        mat.add(0, 0, 3.0)
        mat.add(0, 1, -1.0)
        mat.add(1, 0, -1.0)
        mat.add(1, 1, 3.0)
        mat.add(1, 2, -1.0)
        mat.add(2, 1, -1.0)
        mat.add(2, 2, 3.0)
        mat.fix(False)

        rhs = [2.0, 1.0, 2.0]

        solver = SparseSolvPy.MatSolvers()
        solution, converged = solver.solveICMRTR(
            mat, rhs, conv_cri=1e-8, max_ite=100, accela=1.0
        )

        self.assertTrue(converged)

        # Verify solution is reasonable (multiply_vector not implemented yet)
        # This test could be enhanced when matrix-vector operations are added
        self.assertEqual(len(solution), len(rhs))


@unittest.skipUnless(HAS_PYTHON_BINDINGS, "Python bindings not available")
class TestNumPyIntegration(unittest.TestCase):
    """Test NumPy integration with SparseSolv"""

    @unittest.skipUnless(HAS_PYTHON_BINDINGS, "Python bindings not available")
    def test_numpy_arrays(self):
        """Test using NumPy arrays with SparseSolv"""
        # Create matrix
        mat = SparseSolvPy.SparseMat(3)
        mat.add(0, 0, 2.0)
        mat.add(1, 1, 3.0)
        mat.add(2, 2, 4.0)
        mat.fix(False)

        # Note: multiply_vector method not implemented yet
        # This test will be updated when matrix-vector operations are added

        # For now, just test that the matrix was created correctly
        self.assertTrue(mat.isFixed())

    def test_large_system_numpy(self):
        """Test large system with NumPy arrays"""
        n = 50
        mat = SparseSolvPy.SparseMat(n)

        # Create tridiagonal system
        for i in range(n):
            mat.add(i, i, 4.0)
            if i > 0:
                mat.add(i, i - 1, -1.0)
            if i < n - 1:
                mat.add(i, i + 1, -1.0)
        mat.fix(False)

        # Create RHS with NumPy
        rhs = np.ones(n)

        solver = SparseSolvPy.MatSolvers()
        solver.setDiagScale(True)

        solution, converged = solver.solveICCG(
            mat, rhs.tolist(), conv_cri=1e-8, max_ite=200, accela=1.0
        )

        self.assertTrue(converged)
        self.assertEqual(len(solution), n)


class TestFallbackBehavior(unittest.TestCase):
    """Test behavior when Python bindings are not available"""

    @unittest.skipIf(HAS_PYTHON_BINDINGS, "Python bindings are available")
    def test_graceful_import_failure(self):
        """Test that the test suite handles missing bindings gracefully"""
        # This test should only run when bindings are NOT available
        self.assertFalse(HAS_PYTHON_BINDINGS)
        print("Python bindings test skipped - bindings not compiled")


if __name__ == "__main__":
    # Print information about the test environment
    print(f"Python version: {sys.version}")
    print(f"NumPy available: {'Yes' if 'numpy' in sys.modules else 'No'}")
    print(f"SparseSolvPy available: {'Yes' if HAS_PYTHON_BINDINGS else 'No'}")
    print()

    # Run the tests
    unittest.main(verbosity=2)
