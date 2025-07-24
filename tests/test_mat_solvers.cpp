#include "sparse-solv.h"
#include <cmath>
#include <complex>
#include <gtest/gtest.h>
#include <random>
#include <vector>

using dcomplex = std::complex<double>;

class MatSolversTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Common setup for tests if needed
  }

  void TearDown() override {
    // Common cleanup for tests if needed
  }

  // Helper function to check vector equality within tolerance
  void AssertVectorNear(const std::vector<double> &expected,
                        const std::vector<double> &actual, double tolerance) {
    ASSERT_EQ(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); ++i) {
      ASSERT_NEAR(expected[i], actual[i], tolerance)
          << "Vector element " << i << " mismatch";
    }
  }

  // Helper function to check complex vector equality within tolerance
  void AssertComplexVectorNear(const std::vector<dcomplex> &expected,
                               const std::vector<dcomplex> &actual,
                               double tolerance) {
    ASSERT_EQ(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); ++i) {
      ASSERT_LT(std::abs(expected[i] - actual[i]), tolerance)
          << "Complex vector element " << i << " mismatch";
    }
  }
};

TEST_F(MatSolversTest, SolverCreation) {
  auto solver = SparseSolv::createSolver();
  ASSERT_NE(solver, nullptr);
}

TEST_F(MatSolversTest, ICCGSimpleDiagonal) {
  // Create diagonal matrix: diag(2, 3, 4)
  auto mat = SparseSolv::createSparseMat(3);
  mat->add(0, 0, 2.0);
  mat->add(1, 1, 3.0);
  mat->add(2, 2, 4.0);
  mat->fix();

  // Right-hand side: [4, 6, 8]
  std::vector<double> rhs = {4.0, 6.0, 8.0};
  std::vector<double> solution(3, 0.0);

  auto solver = SparseSolv::createSolver();
  solver->setDiagScale(true);
  solver->setSaveBest(true);

  bool converged = solver->solveICCG(3, 1e-10, 100, 1.0, *mat, rhs, solution);

  ASSERT_TRUE(converged);

  // Expected solution: [2, 2, 2]
  std::vector<double> expected = {2.0, 2.0, 2.0};
  AssertVectorNear(expected, solution, 1e-8);
}

TEST_F(MatSolversTest, ICCGTridiagonalSystem) {
  // Create tridiagonal matrix:
  // [2 -1  0]
  // [-1 2 -1]
  // [0 -1  2]
  auto mat = SparseSolv::createSparseMat(3);
  mat->add(0, 0, 2.0);
  mat->add(0, 1, -1.0);
  mat->add(1, 0, -1.0);
  mat->add(1, 1, 2.0);
  mat->add(1, 2, -1.0);
  mat->add(2, 1, -1.0);
  mat->add(2, 2, 2.0);
  mat->fix();

  // Right-hand side: [1, 0, 1] (solution should be [1, 1, 1])
  std::vector<double> rhs = {1.0, 0.0, 1.0};
  std::vector<double> solution(3, 0.0);

  auto solver = SparseSolv::createSolver();
  bool converged = solver->solveICCG(3, 1e-10, 100, 1.0, *mat, rhs, solution);

  ASSERT_TRUE(converged);

  // Expected solution: [1, 1, 1]
  std::vector<double> expected = {1.0, 1.0, 1.0};
  AssertVectorNear(expected, solution, 1e-6);
}

TEST_F(MatSolversTest, ICCGLargerSystem) {
  const int n = 50;
  auto mat = SparseSolv::createSparseMat(n);

  // Create a well-conditioned positive definite matrix
  for (int i = 0; i < n; i++) {
    mat->add(i, i, 4.0); // Main diagonal
    if (i > 0)
      mat->add(i, i - 1, -1.0); // Lower diagonal
    if (i < n - 1)
      mat->add(i, i + 1, -1.0); // Upper diagonal
  }
  mat->fix();

  // Create RHS such that solution is all ones
  std::vector<double> expected_solution(n, 1.0);
  auto rhs = (*mat) * expected_solution;

  std::vector<double> solution(n, 0.0);

  auto solver = SparseSolv::createSolver();
  solver->setDiagScale(true);
  bool converged = solver->solveICCG(n, 1e-8, 200, 1.0, *mat, rhs, solution);

  ASSERT_TRUE(converged);
  AssertVectorNear(expected_solution, solution, 1e-5);
}

TEST_F(MatSolversTest, ICMRTRSolver) {
  // Create a simple positive definite system
  auto mat = SparseSolv::createSparseMat(4);
  mat->add(0, 0, 4.0);
  mat->add(0, 1, -1.0);
  mat->add(1, 0, -1.0);
  mat->add(1, 1, 4.0);
  mat->add(1, 2, -1.0);
  mat->add(2, 1, -1.0);
  mat->add(2, 2, 4.0);
  mat->add(2, 3, -1.0);
  mat->add(3, 2, -1.0);
  mat->add(3, 3, 4.0);
  mat->fix();

  // Test with known solution
  std::vector<double> expected_solution = {1.0, 2.0, 3.0, 4.0};
  auto rhs = (*mat) * expected_solution;

  std::vector<double> solution(4, 0.0);

  auto solver = SparseSolv::createSolver();
  bool converged = solver->solveICMRTR(4, 1e-10, 100, 1.0, *mat, rhs, solution);

  ASSERT_TRUE(converged);
  AssertVectorNear(expected_solution, solution, 1e-6);
}

TEST_F(MatSolversTest, SGSMRTRSolver) {
  // Create a simple system
  auto mat = SparseSolv::createSparseMat(3);
  mat->add(0, 0, 3.0);
  mat->add(0, 1, -1.0);
  mat->add(1, 0, -1.0);
  mat->add(1, 1, 3.0);
  mat->add(1, 2, -1.0);
  mat->add(2, 1, -1.0);
  mat->add(2, 2, 3.0);
  mat->fix();

  std::vector<double> rhs = {2.0, 1.0, 2.0};
  std::vector<double> solution(3, 0.0);

  auto solver = SparseSolv::createSolver();
  bool converged = solver->solveSGSMRTR(3, 1e-8, 100, 1.0, *mat, rhs, solution);

  ASSERT_TRUE(converged);

  // Verify solution by checking residual
  auto residual_vec = (*mat) * solution;
  for (size_t i = 0; i < residual_vec.size(); ++i) {
    residual_vec[i] -= rhs[i];
  }

  double residual_norm = 0.0;
  for (double r : residual_vec) {
    residual_norm += r * r;
  }
  residual_norm = std::sqrt(residual_norm);

  ASSERT_LT(residual_norm, 1e-6);
}

TEST_F(MatSolversTest, ComplexICCGSolver) {
  // Create complex Hermitian positive definite matrix
  auto matc = SparseSolv::createSparseMatC(3);
  matc->add(0, 0, dcomplex(3.0, 0.0));
  matc->add(0, 1, dcomplex(1.0, -1.0));
  matc->add(1, 0, dcomplex(1.0, 1.0)); // Hermitian conjugate
  matc->add(1, 1, dcomplex(3.0, 0.0));
  matc->add(1, 2, dcomplex(0.0, -2.0));
  matc->add(2, 1, dcomplex(0.0, 2.0)); // Hermitian conjugate
  matc->add(2, 2, dcomplex(3.0, 0.0));
  matc->fix();

  // Known solution
  std::vector<dcomplex> expected_solution = {
      dcomplex(1.0, 0.5), dcomplex(0.5, 1.0), dcomplex(1.5, -0.5)};
  auto rhs = (*matc) * expected_solution;

  std::vector<dcomplex> solution(3, dcomplex(0.0, 0.0));

  auto solver = SparseSolv::createSolver();
  bool converged = solver->solveICCG(3, 1e-8, 100, 1.0, *matc, rhs, solution);

  ASSERT_TRUE(converged);

  // Check solution accuracy
  AssertComplexVectorNear(expected_solution, solution, 1e-5);
}

TEST_F(MatSolversTest, SolverConfiguration) {
  auto solver = SparseSolv::createSolver();

  // Test configuration settings
  solver->setDiagScale(true);
  solver->setSaveBest(true);
  solver->setBadDivVal(1e6);

  // Create a simple test system
  auto mat = SparseSolv::createSparseMat(2);
  mat->add(0, 0, 2.0);
  mat->add(1, 1, 3.0);
  mat->fix();

  std::vector<double> rhs = {2.0, 3.0};
  std::vector<double> solution(2, 0.0);

  bool converged = solver->solveICCG(2, 1e-10, 100, 1.0, *mat, rhs, solution);

  ASSERT_TRUE(converged);

  // Expected solution: [1, 1]
  std::vector<double> expected = {1.0, 1.0};
  AssertVectorNear(expected, solution, 1e-8);
}

TEST_F(MatSolversTest, SolverFailureCases) {
  // Create a non-positive definite matrix (should fail or struggle to converge)
  auto mat = SparseSolv::createSparseMat(3);
  mat->add(0, 0, -1.0); // Negative diagonal element
  mat->add(1, 1, 1.0);
  mat->add(2, 2, 1.0);
  mat->fix();

  std::vector<double> rhs = {1.0, 1.0, 1.0};
  std::vector<double> solution(3, 0.0);

  auto solver = SparseSolv::createSolver();
  solver->setSaveBest(
      true); // Should save best solution even if it doesn't converge

  // This should either not converge or converge very slowly
  bool converged =
      solver->solveICCG(3, 1e-10, 5, 1.0, *mat, rhs, solution, false);

  // We don't assert convergence here since the matrix is problematic
  // Just verify that the solver doesn't crash
  SUCCEED() << "Solver completed (converged: " << (converged ? "yes" : "no")
            << ")";
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
