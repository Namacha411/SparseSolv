#include "sparse-solv.h"
#include "SparseMat.hpp"
#include "MatSolvers.hpp"

using namespace SRLfem;

// C API implementations
extern "C" {
    void* sparse_mat_create(slv_int size) {
        try {
            return new SparseMat(size);
        } catch (...) {
            return nullptr;
        }
    }
    
    void sparse_mat_destroy(void* mat) {
        if (mat) {
            delete static_cast<SparseMat*>(mat);
        }
    }
    
    void sparse_mat_add(void* mat, slv_int row, slv_int col, double val) {
        if (mat) {
            static_cast<SparseMat*>(mat)->add(row, col, val);
        }
    }
    
    void sparse_mat_fix(void* mat) {
        if (mat) {
            static_cast<SparseMat*>(mat)->fix();
        }
    }
    
    void* mat_solvers_create() {
        try {
            return new MatSolvers();
        } catch (...) {
            return nullptr;
        }
    }
    
    void mat_solvers_destroy(void* solver) {
        if (solver) {
            delete static_cast<MatSolvers*>(solver);
        }
    }
    
    bool mat_solvers_iccg(void* solver, slv_int size, double conv_cri, 
                          int max_ite, double accera, void* mat_a, 
                          const double* vec_b, double* results) {
        if (!solver || !mat_a || !vec_b || !results) {
            return false;
        }
        
        try {
            MatSolvers* s = static_cast<MatSolvers*>(solver);
            SparseMat* a = static_cast<SparseMat*>(mat_a);
            return s->solveICCG(size, conv_cri, max_ite, accera, *a, vec_b, results);
        } catch (...) {
            return false;
        }
    }
}

// Convenience functions
namespace SparseSolv {
    std::unique_ptr<SparseMat> createSparseMat(slv_int size) {
        return std::make_unique<SparseMat>(size);
    }
    
    std::unique_ptr<MatSolvers> createSolver() {
        return std::make_unique<MatSolvers>();
    }
}