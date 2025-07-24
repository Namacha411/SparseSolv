#include "sparse-solv.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "SparseSolv Test Package Example\n";
    
    // Test creating a sparse matrix
    auto mat = SparseSolv::createSparseMat(3);
    
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
    
    std::cout << "Created 3x3 sparse matrix successfully\n";
    
    // Test creating a solver
    auto solver = SparseSolv::createSolver();
    
    std::cout << "Created matrix solver successfully\n";
    
    // Test C API
    void* c_mat = sparse_mat_create(3);
    if (c_mat) {
        sparse_mat_add(c_mat, 0, 0, 1.0);
        sparse_mat_add(c_mat, 1, 1, 1.0);
        sparse_mat_add(c_mat, 2, 2, 1.0);
        sparse_mat_fix(c_mat);
        std::cout << "C API matrix creation successful\n";
        sparse_mat_destroy(c_mat);
    }
    
    void* c_solver = mat_solvers_create();
    if (c_solver) {
        std::cout << "C API solver creation successful\n";
        mat_solvers_destroy(c_solver);
    }
    
    std::cout << "SparseSolv test completed successfully!\n";
    return 0;
}
