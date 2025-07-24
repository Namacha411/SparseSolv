#pragma once

#include <vector>
#include <complex>
#include <memory>

#ifdef _WIN32
  #define SPARSE_SOLV_EXPORT __declspec(dllexport)
#else
  #define SPARSE_SOLV_EXPORT
#endif

// Forward declarations
namespace SRLfem {
    class SparseMat;
    class SparseMatC;  
    class MatSolvers;
    using slv_int = size_t;
    using dcomplex = std::complex<double>;
}

// Public C++ API - expose the main classes
#include "../src/SparseMat.hpp"
#include "../src/SparseMatC.hpp"
#include "../src/MatSolvers.hpp"

// C API for language bindings
extern "C" {
    // SparseMat C API
    SPARSE_SOLV_EXPORT void* sparse_mat_create(SRLfem::slv_int size);
    SPARSE_SOLV_EXPORT void sparse_mat_destroy(void* mat);
    SPARSE_SOLV_EXPORT void sparse_mat_add(void* mat, SRLfem::slv_int row, SRLfem::slv_int col, double val);
    SPARSE_SOLV_EXPORT void sparse_mat_fix(void* mat);
    
    // MatSolvers C API
    SPARSE_SOLV_EXPORT void* mat_solvers_create();
    SPARSE_SOLV_EXPORT void mat_solvers_destroy(void* solver);
    SPARSE_SOLV_EXPORT bool mat_solvers_iccg(void* solver, SRLfem::slv_int size, double conv_cri, 
                                              int max_ite, double accera, void* mat_a, 
                                              const double* vec_b, double* results);
}

// Convenience functions
namespace SparseSolv {
    SPARSE_SOLV_EXPORT std::unique_ptr<SRLfem::SparseMat> createSparseMat(SRLfem::slv_int size);
    SPARSE_SOLV_EXPORT std::unique_ptr<SRLfem::SparseMatC> createSparseMatC(SRLfem::slv_int size);
    SPARSE_SOLV_EXPORT std::unique_ptr<SRLfem::MatSolvers> createSolver();
}
