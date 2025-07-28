#include "sparse-solv.h"
#include <cmath>
#include <gtest/gtest.h>
#include <vector>

class SparseMatTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Common setup for tests if needed
  }

  void TearDown() override {
    // Common cleanup for tests if needed
  }
};

TEST_F(SparseMatTest, Creation) {
  auto mat = SparseSolv::createSparseMat(5);
  ASSERT_NE(mat, nullptr);
}

TEST_F(SparseMatTest, BasicOperations) {
  // Create 3x3 matrix
  auto mat = SparseSolv::createSparseMat(3);

  // Add diagonal elements
  mat->add(0, 0, 2.0);
  mat->add(1, 1, 3.0);
  mat->add(2, 2, 4.0);

  // Add off-diagonal elements
  mat->add(0, 1, -1.0);
  mat->add(1, 0, -1.0);
  mat->add(1, 2, -1.5);
  mat->add(2, 1, -1.5);

  // Fix the matrix
  mat->fix();

  ASSERT_TRUE(mat->isFixed());
}

TEST_F(SparseMatTest, MatrixVectorMultiplication) {
  // Create 3x3 identity matrix
  auto mat = SparseSolv::createSparseMat(3);
  mat->add(0, 0, 1.0);
  mat->add(1, 1, 1.0);
  mat->add(2, 2, 1.0);
  mat->fix();

  // Test with std::vector
  std::vector<double> vec = {1.0, 2.0, 3.0};
  auto result = (*mat) * vec;

  ASSERT_DOUBLE_EQ(1.0, result[0]);
  ASSERT_DOUBLE_EQ(2.0, result[1]);
  ASSERT_DOUBLE_EQ(3.0, result[2]);
}

TEST_F(SparseMatTest, TridiagonalSystem) {
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

  // Test multiplication: A * [1, 1, 1]^T should give [1, 0, 1]^T
  std::vector<double> x = {1.0, 1.0, 1.0};
  auto result = (*mat) * x;

  ASSERT_DOUBLE_EQ(1.0, result[0]);
  ASSERT_DOUBLE_EQ(0.0, result[1]);
  ASSERT_DOUBLE_EQ(1.0, result[2]);
}

TEST_F(SparseMatTest, ScalarMultiplication) {
  auto mat = SparseSolv::createSparseMat(2);
  mat->add(0, 0, 1.0);
  mat->add(1, 1, 2.0);
  mat->fix();

  // Test scalar multiplication
  auto scaled_mat = (*mat) * 3.0;

  std::vector<double> vec = {1.0, 1.0};
  auto result = scaled_mat * vec;

  ASSERT_DOUBLE_EQ(3.0, result[0]);
  ASSERT_DOUBLE_EQ(6.0, result[1]);
}

TEST_F(SparseMatTest, Addition) {
  // Create first matrix (identity)
  auto mat1 = SparseSolv::createSparseMat(2);
  mat1->add(0, 0, 1.0);
  mat1->add(1, 1, 1.0);
  mat1->fix();

  // Create second matrix (2*identity)
  auto mat2 = SparseSolv::createSparseMat(2);
  mat2->add(0, 0, 2.0);
  mat2->add(1, 1, 2.0);
  mat2->fix();

  // Test addition
  auto sum_mat = (*mat1) + (*mat2);

  std::vector<double> vec = {1.0, 1.0};
  auto result = sum_mat * vec;

  ASSERT_DOUBLE_EQ(3.0, result[0]);
  ASSERT_DOUBLE_EQ(3.0, result[1]);
}

TEST_F(SparseMatTest, LargeMatrix) {
  const int n = 100;
  auto mat = SparseSolv::createSparseMat(n);

  // Create pentadiagonal matrix
  for (int i = 0; i < n; i++) {
    mat->add(i, i, 4.0); // Main diagonal
    if (i > 0)
      mat->add(i, i - 1, -1.0); // Lower diagonal
    if (i < n - 1)
      mat->add(i, i + 1, -1.0); // Upper diagonal
    if (i > 1)
      mat->add(i, i - 2, -0.5); // Second lower diagonal
    if (i < n - 2)
      mat->add(i, i + 2, -0.5); // Second upper diagonal
  }

  mat->fix();

  // Test with ones vector
  std::vector<double> ones(n, 1.0);
  auto result = (*mat) * ones;

  // For pentadiagonal with these values, result should be predictable
  ASSERT_DOUBLE_EQ(2.5, result[0]); // 4*1 - 1*1 - 0.5*1
  ASSERT_DOUBLE_EQ(1.0, result[1]); // 4*1 - 1*1 - 1*1 - 0.5*1 - 0.5*1
}

TEST_F(SparseMatTest, EmptyOperations) {
  auto mat = SparseSolv::createSparseMat(5);
  ASSERT_FALSE(mat->isFixed());

  // Fix empty matrix
  mat->fix();
  ASSERT_TRUE(mat->isFixed());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}