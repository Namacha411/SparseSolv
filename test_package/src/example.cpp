#include "sparse-solv.h"
#include <gtest/gtest.h>
#include <iostream>
#include <vector>

// C API declarations for test package
extern "C" {
    void* sparse_mat_create(int n);
    void sparse_mat_destroy(void* mat);
    void sparse_mat_add(void* mat, int i, int j, double val);
    void sparse_mat_fix(void* mat);
    
    void* mat_solvers_create();
    void mat_solvers_destroy(void* solver);
}

class TestPackageTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup for each test
    }

    void TearDown() override {
        // Cleanup for each test
    }
};

TEST_F(TestPackageTest, SparseMatCreation) {
    // Test creating a sparse matrix
    auto mat = SparseSolv::createSparseMat(3);
    ASSERT_NE(mat, nullptr);
    
    // Add some entries to the matrix
    // Create a simple 3x3 test matrix:
    // [2 -1  0]
    // [-1 2 -1]
    // [0 -1  2]
    mat->add(0, 0, 2.0);
    mat->add(0, 1, -1.0);
    mat->add(1, 0, -1.0);
    mat->add(1, 1, 2.0);
    mat->add(1, 2, -1.0);
    mat->add(2, 1, -1.0);
    mat->add(2, 2, 2.0);
    
    // Fix the matrix structure
    mat->fix();
    
    ASSERT_TRUE(mat->isFixed());
    std::cout << "Created 3x3 sparse matrix successfully" << std::endl;
}

TEST_F(TestPackageTest, SolverCreation) {
    // Test creating a solver
    auto solver = SparseSolv::createSolver();
    ASSERT_NE(solver, nullptr);
    
    std::cout << "Created matrix solver successfully" << std::endl;
}

TEST_F(TestPackageTest, CAPIBasicFunctionality) {
    // Test C API
    void* c_mat = sparse_mat_create(3);
    ASSERT_NE(c_mat, nullptr);
    
    sparse_mat_add(c_mat, 0, 0, 1.0);
    sparse_mat_add(c_mat, 1, 1, 1.0);
    sparse_mat_add(c_mat, 2, 2, 1.0);
    sparse_mat_fix(c_mat);
    
    std::cout << "C API matrix creation successful" << std::endl;
    sparse_mat_destroy(c_mat);
    
    void* c_solver = mat_solvers_create();
    ASSERT_NE(c_solver, nullptr);
    
    std::cout << "C API solver creation successful" << std::endl;
    mat_solvers_destroy(c_solver);
}

TEST_F(TestPackageTest, IntegrationTest) {
    // Create a simple system and solve it
    auto mat = SparseSolv::createSparseMat(2);
    mat->add(0, 0, 2.0);
    mat->add(1, 1, 3.0);
    mat->fix();
    
    auto solver = SparseSolv::createSolver();
    solver->setDiagScale(true);
    
    std::vector<double> rhs = {4.0, 6.0};
    std::vector<double> solution(2, 0.0);
    
    bool converged = solver->solveICCG(2, 1e-10, 100, 1.0, *mat, rhs, solution);
    
    ASSERT_TRUE(converged);
    ASSERT_NEAR(2.0, solution[0], 1e-8);
    ASSERT_NEAR(2.0, solution[1], 1e-8);
    
    std::cout << "Basic solve test completed successfully" << std::endl;
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