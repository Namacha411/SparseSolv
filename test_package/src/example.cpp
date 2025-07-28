#include "sparse-solv.h"
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

// Test package for SparseSolv using C API
class TestPackageTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup for each test
  }

  void TearDown() override {
    // Cleanup for each test
  }
};

TEST_F(TestPackageTest, CAPIMatrixCreation) {
  // Test creating a sparse matrix using C API
  void *mat = sparse_mat_create(3);
  ASSERT_NE(mat, nullptr);

  // Add some entries to the matrix
  sparse_mat_add(mat, 0, 0, 2.0);
  sparse_mat_add(mat, 0, 1, -1.0);
  sparse_mat_add(mat, 1, 0, -1.0);
  sparse_mat_add(mat, 1, 1, 2.0);
  sparse_mat_add(mat, 1, 2, -1.0);
  sparse_mat_add(mat, 2, 1, -1.0);
  sparse_mat_add(mat, 2, 2, 2.0);

  // Fix the matrix structure
  sparse_mat_fix(mat);

  std::cout << "C API matrix creation successful" << std::endl;
  sparse_mat_destroy(mat);
}

TEST_F(TestPackageTest, CAPISolverCreation) {
  // Test creating a solver using C API
  void *solver = mat_solvers_create();
  ASSERT_NE(solver, nullptr);

  std::cout << "C API solver creation successful" << std::endl;
  mat_solvers_destroy(solver);
}

TEST_F(TestPackageTest, CAPIComplexMatrixCreation) {
  // Test creating a complex sparse matrix using C API
  void *matc = sparse_mat_c_create(2);
  ASSERT_NE(matc, nullptr);

  // Add complex entries
  std::complex<double> val1(2.0, 1.0);
  std::complex<double> val2(1.0, -1.0);
  sparse_mat_c_add(matc, 0, 0, val1);
  sparse_mat_c_add(matc, 1, 1, val2);

  // Fix the matrix structure
  sparse_mat_c_fix(matc);

  std::cout << "C API complex matrix creation successful" << std::endl;
  sparse_mat_c_destroy(matc);
}

TEST_F(TestPackageTest, CAPIIntegrationTest) {
  // Test a simple solve using C API
  void *mat = sparse_mat_create(2);
  ASSERT_NE(mat, nullptr);

  // Create identity matrix
  sparse_mat_add(mat, 0, 0, 1.0);
  sparse_mat_add(mat, 1, 1, 1.0);
  sparse_mat_fix(mat);

  void *solver = mat_solvers_create();
  ASSERT_NE(solver, nullptr);

  mat_solvers_set_diag_scale(solver, true);

  // Simple test data
  double rhs[2] = {2.0, 3.0};
  double solution[2] = {0.0, 0.0};

  // Solve the system
  bool converged =
      mat_solvers_solve_iccg(solver, 2, 1e-10, 100, 1.0, mat, rhs, solution);

  ASSERT_TRUE(converged);
  EXPECT_NEAR(2.0, solution[0], 1e-8);
  EXPECT_NEAR(3.0, solution[1], 1e-8);

  std::cout << "C API integration test completed successfully" << std::endl;

  sparse_mat_destroy(mat);
  mat_solvers_destroy(solver);
}

int main(int argc, char **argv) {
  std::cout << "SparseSolv Test Package with GoogleTest" << std::endl;

  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();

  if (result == 0) {
    std::cout << "SparseSolv test package completed successfully!" << std::endl;
  } else {
    std::cout << "SparseSolv test package failed!" << std::endl;
  }

  return result;
}