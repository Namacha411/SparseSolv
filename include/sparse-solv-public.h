#pragma once

#include <complex>
#include <memory>
#include <vector>

#ifdef _WIN32
#define SPARSE_SOLV_EXPORT __declspec(dllexport)
#else
#define SPARSE_SOLV_EXPORT
#endif

// Forward declarations
namespace SRLfem {
using slv_int = size_t;
using dcomplex = std::complex<double>;

// Forward declare classes
class SparseMat;
class SparseMatC;
class MatSolvers;
} // namespace SRLfem

// C++ convenience namespace with factory functions
namespace SparseSolv {
// Factory functions for creating objects
SPARSE_SOLV_EXPORT std::unique_ptr<SRLfem::SparseMat>
createSparseMat(SRLfem::slv_int size);
SPARSE_SOLV_EXPORT std::unique_ptr<SRLfem::SparseMatC>
createSparseMatC(SRLfem::slv_int size);
SPARSE_SOLV_EXPORT std::unique_ptr<SRLfem::MatSolvers> createSolver();
} // namespace SparseSolv

// C API for language bindings
extern "C" {
// SparseMat C API
SPARSE_SOLV_EXPORT void *sparse_mat_create(SRLfem::slv_int size);
SPARSE_SOLV_EXPORT void sparse_mat_destroy(void *mat);
SPARSE_SOLV_EXPORT void sparse_mat_add(void *mat, SRLfem::slv_int row,
                                       SRLfem::slv_int col, double val);
SPARSE_SOLV_EXPORT void sparse_mat_fix(void *mat);

// SparseMatC C API
SPARSE_SOLV_EXPORT void *sparse_mat_c_create(SRLfem::slv_int size);
SPARSE_SOLV_EXPORT void sparse_mat_c_destroy(void *mat);
SPARSE_SOLV_EXPORT void sparse_mat_c_add(void *mat, SRLfem::slv_int row,
                                         SRLfem::slv_int col,
                                         SRLfem::dcomplex val);
SPARSE_SOLV_EXPORT void sparse_mat_c_fix(void *mat);

// MatSolvers C API
SPARSE_SOLV_EXPORT void *mat_solvers_create();
SPARSE_SOLV_EXPORT void mat_solvers_destroy(void *solver);
SPARSE_SOLV_EXPORT void mat_solvers_set_diag_scale(void *solver, bool enable);
SPARSE_SOLV_EXPORT bool
mat_solvers_solve_iccg(void *solver, SRLfem::slv_int size, double conv_cri,
                       int max_ite, double accela, void *mat, const double *rhs,
                       double *solution);
}