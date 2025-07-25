#include "sparse-solv.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>

// C API functions are already declared in sparse-solv.h

class CAPITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for tests if needed
    }

    void TearDown() override {
        // Common cleanup for tests if needed
    }
};

TEST_F(CAPITest, SparseMatBasic) {
    // Create matrix using C API
    void* mat = sparse_mat_create(3);
    ASSERT_NE(mat, nullptr);
    
    // Add elements
    sparse_mat_add(mat, 0, 0, 2.0);
    sparse_mat_add(mat, 0, 1, -1.0);
    sparse_mat_add(mat, 1, 0, -1.0);
    sparse_mat_add(mat, 1, 1, 2.0);
    sparse_mat_add(mat, 1, 2, -1.0);
    sparse_mat_add(mat, 2, 1, -1.0);
    sparse_mat_add(mat, 2, 2, 2.0);
    
    // Fix matrix
    sparse_mat_fix(mat);
    
    // Clean up
    sparse_mat_destroy(mat);
    
    SUCCEED() << "C API basic operations completed successfully";
}

TEST_F(CAPITest, MatSolversBasic) {
    // Create solver using available C API
    void* solver = mat_solvers_create();
    ASSERT_NE(solver, nullptr);
    
    // Clean up
    mat_solvers_destroy(solver);
    
    SUCCEED() << "C API solver creation and destruction working";
}

TEST_F(CAPITest, ICCGSolve) {
    // Test using the available solve function from the C API
    void* mat = sparse_mat_create(3);
    sparse_mat_add(mat, 0, 0, 2.0);
    sparse_mat_add(mat, 1, 1, 3.0);
    sparse_mat_add(mat, 2, 2, 4.0);
    sparse_mat_fix(mat);
    
    void* solver = mat_solvers_create();
    
    // Set up system: Ax = b using the available C API
    double rhs[3] = {4.0, 6.0, 8.0};
    double solution[3] = {0.0, 0.0, 0.0};
    
    // Use the actual available solver function
    int converged = mat_solvers_iccg(solver, 3, 1e-10, 100, 1.02, mat, rhs, solution);
    
    ASSERT_TRUE(converged);
    
    // Clean up
    sparse_mat_destroy(mat);
    mat_solvers_destroy(solver);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}