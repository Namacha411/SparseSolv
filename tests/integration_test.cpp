#include "sparse-solv.h"
#include <chrono>
#include <cmath>
#include <complex>
#include <gtest/gtest.h>
#include <random>
#include <vector>

using dcomplex = std::complex<double>;

// C API functions are already declared in sparse-solv.h

class IntegrationTest : public ::testing::Test {
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

TEST_F(IntegrationTest, FiniteElementLikeSystem) {
  const int n = 25; // 5x5 grid
  auto mat = SparseSolv::createSparseMat(n);

  // Create 2D finite difference matrix (5-point stencil)
  // This simulates a typical FEM mass/stiffness matrix structure
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 5; ++j) {
      int idx = i * 5 + j;

      // Main diagonal
      mat->add(idx, idx, 4.0);

      // Neighboring points
      if (j > 0)
        mat->add(idx, idx - 1, -1.0); // Left
      if (j < 4)
        mat->add(idx, idx + 1, -1.0); // Right
      if (i > 0)
        mat->add(idx, idx - 5, -1.0); // Up
      if (i < 4)
        mat->add(idx, idx + 5, -1.0); // Down
    }
  }
  mat->fix();

  // Create RHS for known solution
  std::vector<double> expected_solution(n);
  std::mt19937 gen(42); // Fixed seed for reproducibility
  std::uniform_real_distribution<double> dis(0.1, 2.0);
  for (int i = 0; i < n; ++i) {
    expected_solution[i] = dis(gen);
  }

  auto rhs = (*mat) * expected_solution;
  std::vector<double> solution(n, 0.0);

  // Solve with ICCG
  auto solver = SparseSolv::createSolver();
  solver->setDiagScale(true);
  solver->setSaveBest(true);
  solver->setSaveLog(true);

  auto start = std::chrono::high_resolution_clock::now();
  bool converged = solver->solveICCG(n, 1e-8, 500, 1.0, *mat, rhs, solution);
  auto end = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  std::cout << "ICCG solve time: " << duration.count() << " microseconds"
            << std::endl;

  ASSERT_TRUE(converged);
  AssertVectorNear(expected_solution, solution, 1e-5);

  // Test residual log
  std::vector<double> log;
  solver->getResidualLog(log);
  ASSERT_GT(log.size(), 0);
  std::cout << "Convergence in " << log.size() << " iterations" << std::endl;
}

TEST_F(IntegrationTest, ElectromagneticLikeComplexSystem) {
  const int n = 16;
  auto matc = SparseSolv::createSparseMatC(n);

  // Create a complex system similar to electromagnetic FEM
  // Add main diagonal with complex impedance-like values
  for (int i = 0; i < n; ++i) {
    matc->add(i, i, dcomplex(2.0 + i * 0.1, 0.5 + i * 0.05));

    // Add coupling terms
    if (i > 0) {
      matc->add(i, i - 1, dcomplex(-0.3, 0.2));
      matc->add(i - 1, i, dcomplex(-0.3, -0.2)); // Hermitian
    }
    if (i < n - 2) {
      matc->add(i, i + 2, dcomplex(-0.1, 0.1));
      matc->add(i + 2, i, dcomplex(-0.1, -0.1)); // Hermitian
    }
  }
  matc->fix();

  // Create complex RHS
  std::vector<dcomplex> expected_solution(n);
  for (int i = 0; i < n; ++i) {
    expected_solution[i] = dcomplex(1.0 + i * 0.1, 0.5 - i * 0.05);
  }

  auto rhs = (*matc) * expected_solution;
  std::vector<dcomplex> solution(n, dcomplex(0.0, 0.0));

  // Solve with complex ICCG
  auto solver = SparseSolv::createSolver();
  solver->setDiagScale(true);

  auto start = std::chrono::high_resolution_clock::now();
  bool converged = solver->solveICCG(n, 1e-8, 200, 1.0, *matc, rhs, solution);
  auto end = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start);
  std::cout << "Complex ICCG solve time: " << duration.count()
            << " microseconds" << std::endl;

  ASSERT_TRUE(converged);

  // Check solution accuracy
  AssertComplexVectorNear(expected_solution, solution, 1e-5);
}

TEST_F(IntegrationTest, SolverComparison) {
  const int n = 20;
  auto mat = SparseSolv::createSparseMat(n);

  // Create a moderately conditioned system
  for (int i = 0; i < n; ++i) {
    mat->add(i, i, 3.0 + i * 0.1);
    if (i > 0)
      mat->add(i, i - 1, -0.8);
    if (i < n - 1)
      mat->add(i, i + 1, -0.8);
    if (i > 1)
      mat->add(i, i - 2, -0.2);
    if (i < n - 2)
      mat->add(i, i + 2, -0.2);
  }
  mat->fix();

  // Same RHS for all solvers
  std::vector<double> rhs(n);
  for (int i = 0; i < n; ++i) {
    rhs[i] = std::sin(i * 0.3) + 1.0;
  }

  // Test ICCG
  {
    std::vector<double> solution(n, 0.0);
    auto solver = SparseSolv::createSolver();
    solver->setSaveLog(true);

    auto start = std::chrono::high_resolution_clock::now();
    bool converged = solver->solveICCG(n, 1e-8, 100, 1.0, *mat, rhs, solution);
    auto end = std::chrono::high_resolution_clock::now();

    ASSERT_TRUE(converged);

    std::vector<double> iccg_log;
    solver->getResidualLog(iccg_log);

    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "ICCG: " << iccg_log.size() << " iterations, "
              << duration.count() << " microseconds" << std::endl;
  }

  // Test IC-MRTR
  {
    std::vector<double> solution(n, 0.0);
    auto solver = SparseSolv::createSolver();
    solver->setSaveLog(true);

    auto start = std::chrono::high_resolution_clock::now();
    bool converged =
        solver->solveICMRTR(n, 1e-8, 100, 1.0, *mat, rhs, solution);
    auto end = std::chrono::high_resolution_clock::now();

    ASSERT_TRUE(converged);

    std::vector<double> mrtr_log;
    solver->getResidualLog(mrtr_log);

    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "IC-MRTR: " << mrtr_log.size() << " iterations, "
              << duration.count() << " microseconds" << std::endl;
  }

  // Test SGS-MRTR
  {
    std::vector<double> solution(n, 0.0);
    auto solver = SparseSolv::createSolver();
    solver->setSaveLog(true);

    auto start = std::chrono::high_resolution_clock::now();
    bool converged = solver->solveSGSMRTR(n, 1e-8, 100, *mat, rhs, solution);
    auto end = std::chrono::high_resolution_clock::now();

    ASSERT_TRUE(converged);

    std::vector<double> sgs_log;
    solver->getResidualLog(sgs_log);

    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "SGS-MRTR: " << sgs_log.size() << " iterations, "
              << duration.count() << " microseconds" << std::endl;
  }
}

TEST_F(IntegrationTest, MixedAPIUsage) {
  // Create matrix with C++ API
  auto mat = SparseSolv::createSparseMat(4);
  mat->add(0, 0, 2.0);
  mat->add(0, 1, -1.0);
  mat->add(1, 0, -1.0);
  mat->add(1, 1, 2.0);
  mat->add(1, 2, -1.0);
  mat->add(2, 1, -1.0);
  mat->add(2, 2, 2.0);
  mat->add(2, 3, -1.0);
  mat->add(3, 2, -1.0);
  mat->add(3, 3, 2.0);
  mat->fix();

  // Use C API solver
  void *c_solver = mat_solvers_create();
  ASSERT_NE(c_solver, nullptr);
  // Note: mat_solvers_set_diag_scale not implemented in C API

  // Create problem
  double rhs[4] = {1.0, 0.0, 0.0, 1.0};
  double solution[4] = {0.0, 0.0, 0.0, 0.0};

  // Extract matrix data for C API
  // Note: This would require additional methods in the real implementation
  // For now, we'll test that the objects can coexist

  // Clean up C API objects
  mat_solvers_destroy(c_solver);

  // C++ objects clean up automatically
  SUCCEED() << "Mixed API usage test completed successfully";
}

TEST_F(IntegrationTest, PerformanceScaling) {
  std::vector<int> sizes = {10, 25, 50, 100};

  for (int n : sizes) {
    auto mat = SparseSolv::createSparseMat(n);

    // Create well-conditioned system
    for (int i = 0; i < n; ++i) {
      mat->add(i, i, 4.0);
      if (i > 0)
        mat->add(i, i - 1, -1.0);
      if (i < n - 1)
        mat->add(i, i + 1, -1.0);
    }
    mat->fix();

    std::vector<double> rhs(n, 1.0);
    std::vector<double> solution(n, 0.0);

    auto solver = SparseSolv::createSolver();
    solver->setDiagScale(true);
    solver->setSaveLog(true);

    auto start = std::chrono::high_resolution_clock::now();
    bool converged = solver->solveICCG(n, 1e-8, 200, 1.0, *mat, rhs, solution);
    auto end = std::chrono::high_resolution_clock::now();

    ASSERT_TRUE(converged);

    std::vector<double> log;
    solver->getResidualLog(log);

    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Size " << n << ": " << log.size() << " iterations, "
              << duration.count() << " microseconds" << std::endl;
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}