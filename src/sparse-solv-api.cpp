#include "MatSolvers.hpp"
#include "SparseMat.hpp"
#include "SparseMatC.hpp"
#include "sparse-solv.h"

using namespace SRLfem;

// C API implementations
extern "C" {
void *sparse_mat_create(slv_int size) {
  try {
    return new SparseMat(size);
  } catch (...) {
    return nullptr;
  }
}

void sparse_mat_destroy(void *mat) {
  if (mat) {
    delete static_cast<SparseMat *>(mat);
  }
}

void sparse_mat_add(void *mat, slv_int row, slv_int col, double val) {
  if (mat) {
    static_cast<SparseMat *>(mat)->add(row, col, val);
  }
}

void sparse_mat_fix(void *mat) {
  if (mat) {
    static_cast<SparseMat *>(mat)->fix(false);
  }
}

void *mat_solvers_create() {
  try {
    return new MatSolvers();
  } catch (...) {
    return nullptr;
  }
}

void mat_solvers_destroy(void *solver) {
  if (solver) {
    delete static_cast<MatSolvers *>(solver);
  }
}

void mat_solvers_set_diag_scale(void *solver, bool enable) {
  if (solver) {
    static_cast<MatSolvers *>(solver)->setDiagScale(enable);
  }
}

// SparseMatC C API functions
void *sparse_mat_c_create(slv_int size) {
  try {
    return new SparseMatC(size);
  } catch (...) {
    return nullptr;
  }
}

void sparse_mat_c_destroy(void *mat) {
  if (mat) {
    delete static_cast<SparseMatC *>(mat);
  }
}

void sparse_mat_c_add(void *mat, slv_int row, slv_int col, dcomplex val) {
  if (mat) {
    static_cast<SparseMatC *>(mat)->add(row, col, val);
  }
}

void sparse_mat_c_fix(void *mat) {
  if (mat) {
    static_cast<SparseMatC *>(mat)->fix(false);
  }
}

bool mat_solvers_solve_iccg(void *solver, slv_int size, double conv_cri,
                            int max_ite, double accela, void *mat,
                            const double *rhs, double *solution) {
  if (!solver || !mat || !rhs || !solution) {
    return false;
  }

  try {
    MatSolvers *s = static_cast<MatSolvers *>(solver);
    SparseMat *m = static_cast<SparseMat *>(mat);

    // Convert C arrays to vectors
    std::vector<double> vec_rhs(rhs, rhs + size);
    std::vector<double> vec_solution(solution, solution + size);

    bool converged = s->solveICCG(size, conv_cri, max_ite, accela, *m, vec_rhs,
                                  vec_solution);

    // Copy solution back to C array
    for (slv_int i = 0; i < size; ++i) {
      solution[i] = vec_solution[i];
    }

    return converged;
  } catch (...) {
    return false;
  }
}

bool mat_solvers_iccg(void *solver, slv_int size, double conv_cri, int max_ite,
                      double accera, void *mat_a, const double *vec_b,
                      double *results) {
  if (!solver || !mat_a || !vec_b || !results) {
    return false;
  }

  try {
    MatSolvers *s = static_cast<MatSolvers *>(solver);
    SparseMat *a = static_cast<SparseMat *>(mat_a);
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

std::unique_ptr<SparseMatC> createSparseMatC(slv_int size) {
  return std::make_unique<SparseMatC>(size);
}

std::unique_ptr<MatSolvers> createSolver() {
  return std::make_unique<MatSolvers>();
}
} // namespace SparseSolv