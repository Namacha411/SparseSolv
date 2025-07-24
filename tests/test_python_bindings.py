#!/usr/bin/env python3
"""
Comprehensive tests for SparseSolv Python bindings
"""

import sys
import os
import unittest
import numpy as np

# Try to import the SparseSolvPy module
try:
    import SparseSolvPy
    HAS_PYTHON_BINDINGS = True
except ImportError:
    HAS_PYTHON_BINDINGS = False
    print("WARNING: SparseSolvPy module not found. Python binding tests will be skipped.")
    print("Build with SPARSE_SOLV_WITH_PYTHON_BINDINGS=True to enable these tests.")


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
        mat.fix()
        self.assertTrue(mat.isFixed())
    
    def test_matrix_vector_multiplication(self):
        """Test matrix-vector multiplication"""
        # Create identity matrix
        mat = SparseSolvPy.SparseMat(3)
        mat.add(0, 0, 1.0)
        mat.add(1, 1, 1.0)
        mat.add(2, 2, 1.0)
        mat.fix()
        
        # Test with list
        vec = [1.0, 2.0, 3.0]
        result = mat.multiply_vector(vec)
        
        self.assertAlmostEqual(result[0], 1.0, places=10)
        self.assertAlmostEqual(result[1], 2.0, places=10)
        self.assertAlmostEqual(result[2], 3.0, places=10)
    
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
        mat.fix()
        
        # Test multiplication with [1, 1, 1] should give [1, 0, 1]
        vec = [1.0, 1.0, 1.0]
        result = mat.multiply_vector(vec)
        
        self.assertAlmostEqual(result[0], 1.0, places=10)
        self.assertAlmostEqual(result[1], 0.0, places=10)
        self.assertAlmostEqual(result[2], 1.0, places=10)
    
    def test_scalar_operations(self):
        """Test scalar multiplication"""
        mat = SparseSolvPy.SparseMat(2)
        mat.add(0, 0, 2.0)
        mat.add(1, 1, 3.0)
        mat.fix()
        
        # Test scalar multiplication
        scaled_mat = mat.multiply_scalar(2.0)
        
        vec = [1.0, 1.0]
        result = scaled_mat.multiply_vector(vec)
        
        self.assertAlmostEqual(result[0], 4.0, places=10)
        self.assertAlmostEqual(result[1], 6.0, places=10)


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
        matc.fix()
        
        self.assertTrue(matc.isFixed())
    
    def test_complex_identity(self):
        """Test complex identity matrix"""
        matc = SparseSolvPy.SparseMatC(3)
        matc.add(0, 0, 1.0 + 0.0j)
        matc.add(1, 1, 1.0 + 0.0j)
        matc.add(2, 2, 1.0 + 0.0j)
        matc.fix()
        
        # Test with complex vector
        vec = [1.0 + 2.0j, 3.0 - 1.0j, -1.0 + 1.5j]
        result = matc.multiply_vector(vec)
        
        self.assertAlmostEqual(result[0], 1.0 + 2.0j, places=10)
        self.assertAlmostEqual(result[1], 3.0 - 1.0j, places=10)
        self.assertAlmostEqual(result[2], -1.0 + 1.5j, places=10)
    
    def test_purely_imaginary(self):
        """Test purely imaginary matrix elements"""
        matc = SparseSolvPy.SparseMatC(2)
        matc.add(0, 0, 2.0j)
        matc.add(0, 1, -1.0j)
        matc.add(1, 0, 1.0j)
        matc.add(1, 1, 3.0j)
        matc.fix()
        
        # Test with real vector
        vec = [1.0 + 0.0j, 2.0 + 0.0j]
        result = matc.multiply_vector(vec)
        
        # Expected: [2j*1 + (-j)*2, j*1 + 3j*2] = [0, 7j]
        self.assertAlmostEqual(result[0], 0.0 + 0.0j, places=10)
        self.assertAlmostEqual(result[1], 0.0 + 7.0j, places=10)


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
        mat.fix()
        
        # Right-hand side
        rhs = [4.0, 6.0, 8.0]
        
        solver = SparseSolvPy.MatSolvers()
        solver.setDiagScale(True)
        
        # Solve
        solution, converged = solver.solveICCG(mat, rhs, conv_cri=1e-10, max_ite=100, accela=1.0)
        
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
        mat.fix()
        
        # Right-hand side: [1, 0, 1] (solution should be [1, 1, 1])
        rhs = [1.0, 0.0, 1.0]
        
        solver = SparseSolvPy.MatSolvers()
        solution, converged = solver.solveICCG(mat, rhs, conv_cri=1e-10, max_ite=100, accela=1.0)
        
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
        matc.fix()
        
        # Right-hand side
        rhs = [4.0 + 2.0j, 6.0 - 3.0j]
        
        solver = SparseSolvPy.MatSolvers()
        solution, converged = solver.solveComplexICCG(matc, rhs, conv_cri=1e-10, max_ite=100, accela=1.0)
        
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
        mat.fix()
        
        rhs = [1.0, 2.0]
        
        solver = SparseSolvPy.MatSolvers()
        solver.setSaveLog(True)
        
        solution, converged = solver.solveICCG(mat, rhs, conv_cri=1e-10, max_ite=10, accela=1.0)
        
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
        mat.fix()
        
        rhs = [2.0, 1.0, 2.0]
        
        solver = SparseSolvPy.MatSolvers()
        solution, converged = solver.solveICMRTR(mat, rhs, conv_cri=1e-8, max_ite=100, accela=1.0)
        
        self.assertTrue(converged)
        
        # Verify solution by checking residual
        result_vec = mat.multiply_vector(solution)
        residual = sum((result_vec[i] - rhs[i])**2 for i in range(len(rhs)))
        self.assertLess(residual, 1e-10)


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
        mat.fix()
        
        # Use NumPy array
        vec = np.array([1.0, 2.0, 3.0])
        result = mat.multiply_vector(vec.tolist())
        
        # Convert back to NumPy
        result_np = np.array(result)
        expected = np.array([2.0, 6.0, 12.0])
        
        np.testing.assert_array_almost_equal(result_np, expected)
    
    def test_large_system_numpy(self):
        """Test large system with NumPy arrays"""
        n = 50
        mat = SparseSolvPy.SparseMat(n)
        
        # Create tridiagonal system
        for i in range(n):
            mat.add(i, i, 4.0)
            if i > 0:
                mat.add(i, i-1, -1.0)
            if i < n-1:
                mat.add(i, i+1, -1.0)
        mat.fix()
        
        # Create RHS with NumPy
        rhs = np.ones(n)
        
        solver = SparseSolvPy.MatSolvers()
        solver.setDiagScale(True)
        
        solution, converged = solver.solveICCG(mat, rhs.tolist(), conv_cri=1e-8, max_ite=200, accela=1.0)
        
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


if __name__ == '__main__':
    # Print information about the test environment
    print(f"Python version: {sys.version}")
    print(f"NumPy available: {'Yes' if 'numpy' in sys.modules else 'No'}")
    print(f"SparseSolvPy available: {'Yes' if HAS_PYTHON_BINDINGS else 'No'}")
    print()
    
    # Run the tests
    unittest.main(verbosity=2)
