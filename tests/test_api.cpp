#include "sparse-solv.h"
#include <gtest/gtest.h>
#include <vector>
#include <cmath>

// C API functions are declared in sparse-solv.h
extern "C" {
    void* sparse_mat_create(int n);
    void sparse_mat_destroy(void* mat);
    void sparse_mat_add(void* mat, int i, int j, double val);
    void sparse_mat_fix(void* mat);
    int sparse_mat_is_fixed(void* mat);
    void sparse_mat_multiply_vector(void* mat, const double* vec, double* result);
    
    void* sparse_mat_c_create(int n);
    void sparse_mat_c_destroy(void* mat);
    void sparse_mat_c_add(void* mat, int i, int j, double real, double imag);
    void sparse_mat_c_fix(void* mat);
    int sparse_mat_c_is_fixed(void* mat);
    void sparse_mat_c_multiply_vector(void* mat, const double* vec_real, const double* vec_imag, 
                                      double* result_real, double* result_imag);
    
    void* mat_solvers_create();
    void mat_solvers_destroy(void* solver);
    void mat_solvers_set_diag_scale(void* solver, int enable);
    void mat_solvers_set_save_best(void* solver, int enable);
    void mat_solvers_set_save_log(void* solver, int enable);
    int mat_solvers_solve_iccg(void* solver, void* mat, int n, double tol, int max_iter, 
                               double accel, const double* rhs, double* solution);
    int mat_solvers_solve_complex_iccg(void* solver, void* mat, int n, double tol, int max_iter, 
                                       double accel, const double* rhs_real, const double* rhs_imag,
                                       double* solution_real, double* solution_imag);
}

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
    
    // Test if matrix is fixed
    ASSERT_TRUE(sparse_mat_is_fixed(mat));
    
    // Clean up
    sparse_mat_destroy(mat);
}

TEST_F(CAPITest, SparseMatOperations) {
    // Create identity matrix
    void* mat = sparse_mat_create(3);
    sparse_mat_add(mat, 0, 0, 1.0);
    sparse_mat_add(mat, 1, 1, 1.0);
    sparse_mat_add(mat, 2, 2, 1.0);
    sparse_mat_fix(mat);
    
    // Test matrix-vector multiplication
    double vec[3] = {1.0, 2.0, 3.0};
    double result[3];
    
    sparse_mat_multiply_vector(mat, vec, result);
    
    // Should get the same vector back (identity matrix)
    ASSERT_DOUBLE_EQ(1.0, result[0]);
    ASSERT_DOUBLE_EQ(2.0, result[1]);
    ASSERT_DOUBLE_EQ(3.0, result[2]);
    
    sparse_mat_destroy(mat);
}

TEST_F(CAPITest, SparseMatCBasic) {
    // Create complex matrix
    void* matc = sparse_mat_c_create(2);
    ASSERT_NE(matc, nullptr);
    
    // Add complex elements (real, imag)
    sparse_mat_c_add(matc, 0, 0, 2.0, 1.0);   // 2+i
    sparse_mat_c_add(matc, 1, 1, 3.0, -1.0);  // 3-i
    
    // Fix matrix
    sparse_mat_c_fix(matc);
    
    ASSERT_TRUE(sparse_mat_c_is_fixed(matc));
    
    sparse_mat_c_destroy(matc);
}

TEST_F(CAPITest, SparseMatCOperations) {
    // Create complex identity matrix
    void* matc = sparse_mat_c_create(2);
    sparse_mat_c_add(matc, 0, 0, 1.0, 0.0);  // 1+0i
    sparse_mat_c_add(matc, 1, 1, 1.0, 0.0);  // 1+0i
    sparse_mat_c_fix(matc);
    
    // Test complex matrix-vector multiplication
    double vec_real[2] = {1.0, 2.0};
    double vec_imag[2] = {0.5, -0.5};
    double result_real[2], result_imag[2];
    
    sparse_mat_c_multiply_vector(matc, vec_real, vec_imag, result_real, result_imag);
    
    // Should get the same vector back (identity matrix)
    ASSERT_DOUBLE_EQ(1.0, result_real[0]);
    ASSERT_DOUBLE_EQ(0.5, result_imag[0]);
    ASSERT_DOUBLE_EQ(2.0, result_real[1]);
    ASSERT_DOUBLE_EQ(-0.5, result_imag[1]);
    
    sparse_mat_c_destroy(matc);
}

TEST_F(CAPITest, MatSolversBasic) {
    // Create solver
    void* solver = mat_solvers_create();
    ASSERT_NE(solver, nullptr);
    
    // Test configuration methods
    mat_solvers_set_diag_scale(solver, 1);  // true
    mat_solvers_set_save_best(solver, 1);   // true
    mat_solvers_set_save_log(solver, 1);    // true
    
    mat_solvers_destroy(solver);
}

TEST_F(CAPITest, ICCGSolve) {
    // Create diagonal matrix
    void* mat = sparse_mat_create(3);
    sparse_mat_add(mat, 0, 0, 2.0);
    sparse_mat_add(mat, 1, 1, 3.0);
    sparse_mat_add(mat, 2, 2, 4.0);
    sparse_mat_fix(mat);
    
    // Create solver
    void* solver = mat_solvers_create();
    mat_solvers_set_diag_scale(solver, 1);
    
    // Set up system: Ax = b
    double rhs[3] = {4.0, 6.0, 8.0};  // b vector
    double solution[3] = {0.0, 0.0, 0.0};  // x vector (initial guess)
    
    // Solve system
    int converged = mat_solvers_solve_iccg(solver, mat, 3, 1e-10, 100, 1.0, rhs, solution);
    
    ASSERT_TRUE(converged);
    
    // Check solution: should be [2, 2, 2]
    ASSERT_NEAR(2.0, solution[0], 1e-8);
    ASSERT_NEAR(2.0, solution[1], 1e-8);
    ASSERT_NEAR(2.0, solution[2], 1e-8);
    
    sparse_mat_destroy(mat);
    mat_solvers_destroy(solver);
}

TEST_F(CAPITest, ComplexICCGSolve) {
    // Create complex diagonal matrix
    void* matc = sparse_mat_c_create(2);
    sparse_mat_c_add(matc, 0, 0, 2.0, 0.0);  // 2+0i
    sparse_mat_c_add(matc, 1, 1, 3.0, 0.0);  // 3+0i
    sparse_mat_c_fix(matc);
    
    // Create solver
    void* solver = mat_solvers_create();
    
    // Set up system: Ax = b
    double rhs_real[2] = {4.0, 6.0};
    double rhs_imag[2] = {2.0, -3.0};
    double solution_real[2] = {0.0, 0.0};
    double solution_imag[2] = {0.0, 0.0};
    
    // Solve system
    int converged = mat_solvers_solve_complex_iccg(solver, matc, 2, 1e-10, 100, 1.0, 
                                                   rhs_real, rhs_imag, solution_real, solution_imag);
    
    ASSERT_TRUE(converged);
    
    // Check solution: [4+2i]/2 = [2+i], [6-3i]/3 = [2-i]
    ASSERT_NEAR(2.0, solution_real[0], 1e-8);
    ASSERT_NEAR(1.0, solution_imag[0], 1e-8);
    ASSERT_NEAR(2.0, solution_real[1], 1e-8);
    ASSERT_NEAR(-1.0, solution_imag[1], 1e-8);
    
    sparse_mat_c_destroy(matc);
    mat_solvers_destroy(solver);
}

TEST_F(CAPITest, ErrorHandling) {
    // Test null pointer handling
    ASSERT_FALSE(sparse_mat_is_fixed(nullptr));
    ASSERT_FALSE(sparse_mat_c_is_fixed(nullptr));
    
    // Test operations on unfixed matrix
    void* mat = sparse_mat_create(2);
    ASSERT_FALSE(sparse_mat_is_fixed(mat));
    
    // Clean up
    sparse_mat_destroy(mat);
}

TEST_F(CAPITest, MemoryManagement) {
    // Create and destroy multiple objects to test for memory leaks
    for (int i = 0; i < 10; ++i) {
        void* mat = sparse_mat_create(100);
        sparse_mat_add(mat, 0, 0, 1.0);
        sparse_mat_add(mat, 50, 50, 2.0);
        sparse_mat_add(mat, 99, 99, 3.0);
        sparse_mat_fix(mat);
        sparse_mat_destroy(mat);
        
        void* matc = sparse_mat_c_create(50);
        sparse_mat_c_add(matc, 0, 0, 1.0, 0.0);
        sparse_mat_c_add(matc, 25, 25, 2.0, 1.0);
        sparse_mat_c_fix(matc);
        sparse_mat_c_destroy(matc);
        
        void* solver = mat_solvers_create();
        mat_solvers_set_diag_scale(solver, 1);
        mat_solvers_destroy(solver);
    }
    
    // If we get here without crashes, memory management is working
    SUCCEED();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}