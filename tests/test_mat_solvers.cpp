#include "sparse-solv.h"
#include <chrono>
#include <cmath>
#include <complex>
#include <gtest/gtest.h>
#include <random>
#include <unsupported/Eigen/SparseExtra>
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
  bool converged = solver->solveSGSMRTR(3, 1e-8, 100, *mat, rhs, solution);

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

// ABMC-ICCG Test Fixture
class ABMCICCGTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Common setup for ABMC-ICCG tests
  }

  void TearDown() override {
    // Common cleanup for ABMC-ICCG tests
  }

  // Helper function to load Matrix Market file and convert to SparseSolv format
  // Returns a pair of (matrix, size)
  std::pair<std::unique_ptr<SRLfem::SparseMat>, int>
  loadMatrixMarketFile(const std::string &filename) {
    // Load matrix using Eigen's loadMarket function
    Eigen::SparseMatrix<double> eigen_mat;
    std::string full_path = "../../tests/test_matrix/" + filename;

    bool loaded = Eigen::loadMarket(eigen_mat, full_path);
    if (!loaded) {
      throw std::runtime_error("Failed to load matrix file: " + full_path);
    }

    int matrix_size = eigen_mat.rows();

    // Convert to SparseSolv format
    auto sparse_mat = SparseSolv::createSparseMat(matrix_size);

    // Iterate through non-zeros and add to SparseMat
    for (int k = 0; k < eigen_mat.outerSize(); ++k) {
      for (Eigen::SparseMatrix<double>::InnerIterator it(eigen_mat, k); it;
           ++it) {
        sparse_mat->add(it.row(), it.col(), it.value());
        // Add symmetric entry if not on diagonal (Matrix Market symmetric
        // format)
        if (it.row() != it.col()) {
          sparse_mat->add(it.col(), it.row(), it.value());
        }
      }
    }

    sparse_mat->fix();
    return std::make_pair(std::move(sparse_mat), matrix_size);
  }

  // Helper function to create a right-hand side vector with known solution
  std::vector<double>
  createRHSWithKnownSolution(const SRLfem::SparseMat &mat,
                             const std::vector<double> &known_solution) {
    return mat * known_solution;
  }

  // Helper function to calculate relative error between two vectors
  double calculateRelativeError(const std::vector<double> &vec1,
                                const std::vector<double> &vec2) {
    EXPECT_EQ(vec1.size(), vec2.size());

    double diff_norm = 0.0;
    double vec2_norm = 0.0;

    for (size_t i = 0; i < vec1.size(); ++i) {
      double diff = vec1[i] - vec2[i];
      diff_norm += diff * diff;
      vec2_norm += vec2[i] * vec2[i];
    }

    diff_norm = std::sqrt(diff_norm);
    vec2_norm = std::sqrt(vec2_norm);

    return (vec2_norm > 1e-15) ? diff_norm / vec2_norm : diff_norm;
  }

  // Helper function to calculate residual error ||Ax - b||_2 / ||b||_2
  double calculateResidualError(const SRLfem::SparseMat &mat,
                                const std::vector<double> &solution,
                                const std::vector<double> &rhs) {
    auto residual = mat * solution;

    double residual_norm = 0.0;
    double rhs_norm = 0.0;

    for (size_t i = 0; i < residual.size(); ++i) {
      double r = residual[i] - rhs[i];
      residual_norm += r * r;
      rhs_norm += rhs[i] * rhs[i];
    }

    residual_norm = std::sqrt(residual_norm);
    rhs_norm = std::sqrt(rhs_norm);

    return (rhs_norm > 1e-15) ? residual_norm / rhs_norm : residual_norm;
  }
};

TEST_F(ABMCICCGTest, BasicFunctionalityWithSmallMatrix) {
  // Test basic ABMC-ICCG functionality with bcsstk04.mtx (132x132 matrix)
  try {
    auto matrix_pair = loadMatrixMarketFile("bcsstk04.mtx");
    auto matrix = std::move(matrix_pair.first);
    const int matrix_size = matrix_pair.second;

    // Create a known solution vector (all ones for simplicity)
    std::vector<double> known_solution(matrix_size, 1.0);

    // Create RHS vector: b = A * x_known
    auto rhs = createRHSWithKnownSolution(*matrix, known_solution);

    // Prepare solution vectors for both methods
    std::vector<double> solution_abmc(matrix_size, 0.0);
    std::vector<double> solution_iccg(matrix_size, 0.0);

    // Solver configuration
    auto solver = SparseSolv::createSolver();
    solver->setDiagScale(true);
    solver->setSaveBest(true);

    const double conv_cri = 1e-8;
    const int max_iter = 500;
    const double accera = 1.0;
    const int num_blocks = 4;
    const int num_colors = 8;

    // Solve with regular ICCG first
    bool iccg_converged = solver->solveICCG(
        matrix_size, conv_cri, max_iter, accera, *matrix, rhs, solution_iccg);
    ASSERT_TRUE(iccg_converged) << "Regular ICCG failed to converge";

    // Solve with ABMC-ICCG
    bool abmc_converged = solver->solveICCGwithABMC(
        matrix_size, conv_cri, max_iter, accera, *matrix, rhs, solution_abmc,
        num_blocks, num_colors);
    ASSERT_TRUE(abmc_converged) << "ABMC-ICCG failed to converge";

    // Verify both methods produce similar results (relaxed tolerance for
    // parallel algorithm)
    double relative_error =
        calculateRelativeError(solution_abmc, solution_iccg);
    EXPECT_LT(relative_error, 1e-5)
        << "ABMC-ICCG solution differs too much from ICCG solution";

    // Verify residual error for ABMC solution
    double residual_error = calculateResidualError(*matrix, solution_abmc, rhs);
    EXPECT_LT(residual_error, 1e-5) << "ABMC-ICCG residual error too large";

    // Verify the solution is close to the known solution
    double solution_error =
        calculateRelativeError(solution_abmc, known_solution);
    EXPECT_LT(solution_error, 1e-3)
        << "ABMC-ICCG solution too far from known solution";

  } catch (const std::exception &e) {
    FAIL() << "Exception in ABMC-ICCG basic test: " << e.what();
  }
}

TEST_F(ABMCICCGTest, AccuracyComparisonWithMultipleMatrices) {
  // Test ABMC-ICCG accuracy against regular ICCG with multiple matrices
  std::vector<std::string> test_matrices = {"bcsstk04.mtx", "mesh1e1.mtx"};

  for (const auto &matrix_file : test_matrices) {
    SCOPED_TRACE("Testing matrix: " + matrix_file);

    try {
      auto matrix_pair = loadMatrixMarketFile(matrix_file);
      auto matrix = std::move(matrix_pair.first);
      const int matrix_size = matrix_pair.second;

      // Create a more complex known solution (linear gradient)
      std::vector<double> known_solution(matrix_size);
      for (int i = 0; i < matrix_size; ++i) {
        known_solution[i] = 1.0 + 0.1 * i / matrix_size;
      }

      // Create RHS vector
      auto rhs = createRHSWithKnownSolution(*matrix, known_solution);

      // Prepare solution vectors
      std::vector<double> solution_abmc(matrix_size, 0.0);
      std::vector<double> solution_iccg(matrix_size, 0.0);

      // Solver configuration
      auto solver = SparseSolv::createSolver();
      solver->setDiagScale(true);
      solver->setSaveBest(true);

      const double conv_cri = 1e-10;
      const int max_iter = 1000;
      const double accera = 1.0;
      const int num_blocks = 6;
      const int num_colors = 12;

      // Solve with regular ICCG
      bool iccg_converged = solver->solveICCG(
          matrix_size, conv_cri, max_iter, accera, *matrix, rhs, solution_iccg);
      ASSERT_TRUE(iccg_converged)
          << "Regular ICCG failed to converge for " << matrix_file;

      // Solve with ABMC-ICCG
      bool abmc_converged = solver->solveICCGwithABMC(
          matrix_size, conv_cri, max_iter, accera, *matrix, rhs, solution_abmc,
          num_blocks, num_colors);
      ASSERT_TRUE(abmc_converged)
          << "ABMC-ICCG failed to converge for " << matrix_file;

      // Compare solutions - they should be reasonably close (parallel algorithm
      // tolerance)
      double relative_error =
          calculateRelativeError(solution_abmc, solution_iccg);
      EXPECT_LT(relative_error, 1e-5)
          << "Solutions differ too much for " << matrix_file
          << " (relative error: " << relative_error << ")";

      // Verify residual errors for both solutions
      double residual_error_iccg =
          calculateResidualError(*matrix, solution_iccg, rhs);
      double residual_error_abmc =
          calculateResidualError(*matrix, solution_abmc, rhs);

      EXPECT_LT(residual_error_iccg, 1e-6)
          << "ICCG residual error too large for " << matrix_file;
      EXPECT_LT(residual_error_abmc, 1e-6)
          << "ABMC-ICCG residual error too large for " << matrix_file;

      // The residual errors should be comparable
      double residual_ratio =
          std::abs(residual_error_abmc - residual_error_iccg) /
          std::max(residual_error_iccg, 1e-15);
      EXPECT_LT(residual_ratio, 100.0)
          << "Residual errors differ too much for " << matrix_file;

    } catch (const std::exception &e) {
      FAIL() << "Exception in accuracy comparison test for " << matrix_file
             << ": " << e.what();
    }
  }
}

TEST_F(ABMCICCGTest, PerformanceValidationWithLargerMatrix) {
  // Test ABMC-ICCG performance and accuracy with larger matrix (bcsstk34.mtx)
  try {
    auto matrix_pair = loadMatrixMarketFile("bcsstk34.mtx");
    auto matrix = std::move(matrix_pair.first);
    const int matrix_size = matrix_pair.second;

    // Create a random-like but reproducible solution
    std::vector<double> known_solution(matrix_size);
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::uniform_real_distribution<double> dis(0.5, 2.0);
    for (int i = 0; i < matrix_size; ++i) {
      known_solution[i] = dis(gen);
    }

    // Create RHS vector
    auto rhs = createRHSWithKnownSolution(*matrix, known_solution);

    // Prepare solution vectors
    std::vector<double> solution_abmc(matrix_size, 0.0);
    std::vector<double> solution_iccg(matrix_size, 0.0);

    // Solver configuration for larger system
    auto solver = SparseSolv::createSolver();
    solver->setDiagScale(true);
    solver->setSaveBest(true);

    const double conv_cri = 1e-8;
    const int max_iter = 2000;
    const double accera = 1.0;
    const int num_blocks = 8;  // More blocks for larger matrix
    const int num_colors = 16; // More colors for better parallelization

    // Measure time for regular ICCG
    auto start_iccg = std::chrono::high_resolution_clock::now();
    bool iccg_converged = solver->solveICCG(
        matrix_size, conv_cri, max_iter, accera, *matrix, rhs, solution_iccg);
    auto end_iccg = std::chrono::high_resolution_clock::now();
    auto duration_iccg = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_iccg - start_iccg);

    ASSERT_TRUE(iccg_converged)
        << "Regular ICCG failed to converge for bcsstk34.mtx";

    // Measure time for ABMC-ICCG
    auto start_abmc = std::chrono::high_resolution_clock::now();
    bool abmc_converged = solver->solveICCGwithABMC(
        matrix_size, conv_cri, max_iter, accera, *matrix, rhs, solution_abmc,
        num_blocks, num_colors);
    auto end_abmc = std::chrono::high_resolution_clock::now();
    auto duration_abmc = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_abmc - start_abmc);

    ASSERT_TRUE(abmc_converged)
        << "ABMC-ICCG failed to converge for bcsstk34.mtx";

    // Verify solution accuracy
    double relative_error =
        calculateRelativeError(solution_abmc, solution_iccg);
    EXPECT_LT(relative_error, 1e-6)
        << "ABMC-ICCG solution accuracy insufficient for large matrix"
        << " (relative error: " << relative_error << ")";

    // Verify residual errors
    double residual_error_iccg =
        calculateResidualError(*matrix, solution_iccg, rhs);
    double residual_error_abmc =
        calculateResidualError(*matrix, solution_abmc, rhs);

    EXPECT_LT(residual_error_iccg, 1e-6)
        << "ICCG residual error too large for bcsstk34.mtx";
    EXPECT_LT(residual_error_abmc, 1e-6)
        << "ABMC-ICCG residual error too large for bcsstk34.mtx";

    // Log performance information (this will be visible in verbose test output)
    std::cout << "Performance comparison for bcsstk34.mtx (size: "
              << matrix_size << "):\n";
    std::cout << "  ICCG time: " << duration_iccg.count() << " ms\n";
    std::cout << "  ABMC-ICCG time: " << duration_abmc.count() << " ms\n";
    std::cout << "  Relative error: " << relative_error << "\n";
    std::cout << "  iccg converged: " << iccg_converged << "\n";
    std::cout << "  abmc converged: " << abmc_converged << "\n";
    std::cout << "  iccg residual: " << residual_error_iccg << "\n";
    std::cout << "  abmc residual: " << residual_error_abmc << "\n";

    // Both methods should complete in reasonable time (less than 30 seconds for
    // this size)
    EXPECT_LT(duration_iccg.count(), 30000) << "ICCG took too long";
    EXPECT_LT(duration_abmc.count(), 30000) << "ABMC-ICCG took too long";

  } catch (const std::exception &e) {
    FAIL() << "Exception in performance validation test: " << e.what();
  }
}

TEST_F(ABMCICCGTest, ParameterSensitivityTest) {
  // Test ABMC-ICCG with different num_blocks and num_colors parameters
  try {
    auto matrix_pair = loadMatrixMarketFile("bcsstk04.mtx");
    auto matrix = std::move(matrix_pair.first);
    const int matrix_size = matrix_pair.second;

    // Create known solution
    std::vector<double> known_solution(matrix_size, 1.0);
    auto rhs = createRHSWithKnownSolution(*matrix, known_solution);

    // Test different parameter combinations
    std::vector<std::pair<int, int>> param_combinations = {
        {2, 4},  // Small blocks, few colors
        {4, 8},  // Medium blocks, medium colors
        {6, 12}, // More blocks, more colors
        {8, 16}  // Many blocks, many colors
    };

    // Reference solution using regular ICCG
    std::vector<double> reference_solution(matrix_size, 0.0);
    auto solver_ref = SparseSolv::createSolver();
    solver_ref->setDiagScale(true);
    solver_ref->setSaveBest(true);

    const double conv_cri = 1e-8;
    const int max_iter = 500;
    const double accera = 1.0;

    // Measure time for ICCG
    auto start_iccg = std::chrono::high_resolution_clock::now();
    bool ref_converged =
        solver_ref->solveICCG(matrix_size, conv_cri, max_iter, accera, *matrix,
                              rhs, reference_solution);
    auto end_iccg = std::chrono::high_resolution_clock::now();
    auto duration_iccg = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_iccg - start_iccg);
    ASSERT_TRUE(ref_converged) << "Reference ICCG solution failed";

    // Test each parameter combination
    for (const auto &params : param_combinations) {
      int num_blocks = params.first;
      int num_colors = params.second;

      SCOPED_TRACE("Testing num_blocks=" + std::to_string(num_blocks) +
                   ", num_colors=" + std::to_string(num_colors));

      std::vector<double> solution_abmc(matrix_size, 0.0);
      auto solver = SparseSolv::createSolver();
      solver->setDiagScale(true);
      solver->setSaveBest(true);

      // Measure time for ABMC-ICCG
      auto start_abmc = std::chrono::high_resolution_clock::now();
      bool abmc_converged = solver->solveICCGwithABMC(
          matrix_size, conv_cri, max_iter, accera, *matrix, rhs, solution_abmc,
          num_blocks, num_colors);
      auto end_abmc = std::chrono::high_resolution_clock::now();
      auto duration_abmc =
          std::chrono::duration_cast<std::chrono::milliseconds>(end_abmc -
                                                                start_abmc);

      ASSERT_TRUE(abmc_converged)
          << "ABMC-ICCG failed to converge with params (" << num_blocks << ", "
          << num_colors << ")";

      // Check accuracy against reference solution (relaxed for parallel
      // algorithm)
      double relative_error =
          calculateRelativeError(solution_abmc, reference_solution);
      EXPECT_LT(relative_error, 1e-5)
          << "Solution accuracy insufficient for params (" << num_blocks << ", "
          << num_colors << "), relative error: " << relative_error;

      // Check residual error
      double residual_error =
          calculateResidualError(*matrix, solution_abmc, rhs);
      EXPECT_LT(residual_error, 1e-5)
          << "Residual error too large for params (" << num_blocks << ", "
          << num_colors << "), residual: " << residual_error;

      // Log parameter performance (visible in verbose output)
      std::cout << "Params (" << num_blocks << ", " << num_colors
                << "): relative_error=" << relative_error
                << ", residual=" << residual_error
                << ", time=" << duration_abmc.count() << "\n";
    }

  } catch (const std::exception &e) {
    FAIL() << "Exception in parameter sensitivity test: " << e.what();
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
