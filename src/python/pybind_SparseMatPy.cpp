#include <memory>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../MatSolvers.hpp"
#include "../SparseMat.hpp"

using namespace SRLfem;
namespace py = pybind11;

// Helper functions for modern Python API
namespace {
std::pair<std::vector<double>, bool>
solve_iccg_modern(MatSolvers &solver, const SparseMat &mat,
                  const std::vector<double> &rhs, double conv_cri = 1e-6,
                  int max_ite = 10000, double accela = 1.0, bool init = false) {

  std::vector<double> solution(rhs.size(), 0.0);
  bool converged = solver.solveICCG(rhs.size(), conv_cri, max_ite, accela, mat,
                                    rhs, solution, init);
  return std::make_pair(solution, converged);
}

std::pair<std::vector<std::complex<double>>, bool>
solve_complex_iccg_modern(MatSolvers &solver, const SparseMatC &mat,
                          const std::vector<std::complex<double>> &rhs,
                          double conv_cri = 1e-6, int max_ite = 10000,
                          double accela = 1.0, bool init = false) {

  std::vector<std::complex<double>> solution(rhs.size(),
                                             std::complex<double>(0.0, 0.0));
  bool converged = solver.solveICCG(rhs.size(), conv_cri, max_ite, accela, mat,
                                    rhs, solution, init);
  return std::make_pair(solution, converged);
}

std::pair<std::vector<double>, bool>
solve_icmrtr_modern(MatSolvers &solver, const SparseMat &mat,
                    const std::vector<double> &rhs, double conv_cri = 1e-6,
                    int max_ite = 10000, double accela = 1.0,
                    bool init = false) {

  std::vector<double> solution(rhs.size(), 0.0);
  bool converged = solver.solveICMRTR(rhs.size(), conv_cri, max_ite, accela,
                                      mat, rhs, solution, init);
  return std::make_pair(solution, converged);
}

std::pair<std::vector<double>, bool>
solve_sgsmrtr_modern(MatSolvers &solver, const SparseMat &mat,
                     const std::vector<double> &rhs, double conv_cri = 1e-6,
                     int max_ite = 10000, bool init = false) {

  std::vector<double> solution(rhs.size(), 0.0);
  bool converged = solver.solveSGSMRTR(rhs.size(), conv_cri, max_ite, mat, rhs,
                                       solution, init);
  return std::make_pair(solution, converged);
}

std::pair<std::vector<double>, bool>
solve_abmc_iccg_modern(MatSolvers &solver, const SparseMat &mat,
                       const std::vector<double> &rhs, double conv_cri = 1e-6,
                       int max_ite = 10000, double accela = 1.0,
                       int num_blocks = 250, int num_colors = 4,
                       bool init = false) {
  std::vector<double> solution(rhs.size(), 0.0);
  bool converged = solver.solveICCGwithABMC(rhs.size(), conv_cri, max_ite,
                                            accela, mat, rhs, solution,
                                            num_blocks, num_colors, init);
  return std::make_pair(solution, converged);
}

std::vector<double> multiply_vector_helper(const SparseMat &mat,
                                           const std::vector<double> &vec) {
  std::vector<double> result;
  // We would need to expose matrix-vector multiplication from SparseMat
  // This is a placeholder - actual implementation would depend on available
  // methods
  return result;
}

SparseMat multiply_scalar_helper(const SparseMat &mat, double scalar) {
  // This would need to be implemented in the SparseMat class
  // This is a placeholder
  return mat;
}
} // namespace

PYBIND11_MODULE(SparseSolvPy, m) {
  m.doc() = "SparseSolv Python bindings";

  // SparseMat class
  py::class_<SparseMat>(m, "SparseMat")
      .def(py::init<slv_int>(), "Create sparse matrix with given size")
      .def(
          py::init<slv_int, const std::vector<slv_int> &,
                   const std::vector<slv_int> &, const std::vector<double> &>(),
          "Create sparse matrix from COO format")
      .def("add", &SparseMat::add, "Add entry to matrix")
      .def("resetMat", &SparseMat::resetMat, "Reset matrix")
      .def("fix", &SparseMat::fix, "Finalize matrix structure")
      .def("isFixed", &SparseMat::isFixed, "Check if matrix is finalized")
      .def("delFlagPosition", &SparseMat::delFlagPosition,
           "Delete flag position")
      .def("DiagFlagPosition", &SparseMat::DiagFlagPosition,
           "Diagonal flag position")
      .def("print", &SparseMat::print, "Print matrix")
      .def("printMat", &SparseMat::printMat, "Print matrix to file")
      .def("multiply_vector", &multiply_vector_helper,
           "Matrix-vector multiplication")
      .def("multiply_scalar", &multiply_scalar_helper,
           "Matrix-scalar multiplication");

  // SparseMatC class
  py::class_<SparseMatC>(m, "SparseMatC")
      .def(py::init<slv_int>(), "Create complex sparse matrix with given size")
      .def(py::init<slv_int, const std::vector<slv_int> &,
                    const std::vector<slv_int> &,
                    const std::vector<dcomplex> &>(),
           "Create complex sparse matrix from COO format")
      .def("add", &SparseMatC::add, "Add entry to matrix")
      .def("resetMat", &SparseMatC::resetMat, "Reset matrix")
      .def("fix", &SparseMatC::fix, "Finalize matrix structure")
      .def("isFixed", &SparseMatC::isFixed, "Check if matrix is finalized")
      .def("delFlagPosition", &SparseMatC::delFlagPosition,
           "Delete flag position")
      .def("DiagFlagPosition", &SparseMatC::DiagFlagPosition,
           "Diagonal flag position")
      .def("print", &SparseMatC::print, "Print matrix")
      .def("printMat", &SparseMatC::printMat, "Print matrix to file");

  // MatSolvers class
  py::class_<MatSolvers>(m, "MatSolvers")
      .def(py::init<>(), "Create matrix solver")
      .def("setDiagScale", &MatSolvers::setDiagScale, "Set diagonal scaling")
      .def("setSaveBest", &MatSolvers::setSaveBest, "Set save best solution")
      .def("setSaveLog", &MatSolvers::setSaveLog, "Set save residual log")
      .def("getResidualLog", &MatSolvers::getResidualLog, "Get residual log")
      .def("setDivergeType", &MatSolvers::setDirvegeType, "Set divergence type")
      .def("setBadDivVal", &MatSolvers::setBadDivVal,
           "Set bad divergence value")
      .def("setBadDivCount", &MatSolvers::setBadDivCount,
           "Set bad divergence count")

      // Modern solver interfaces
      .def("solveICCG", &solve_iccg_modern, "Solve using ICCG method",
           py::arg("mat"), py::arg("rhs"), py::arg("conv_cri") = 1e-6,
           py::arg("max_ite") = 10000, py::arg("accela") = 1.0,
           py::arg("init") = false)
      .def("solveComplexICCG", &solve_complex_iccg_modern,
           "Solve complex system using ICCG method", py::arg("mat"),
           py::arg("rhs"), py::arg("conv_cri") = 1e-6,
           py::arg("max_ite") = 10000, py::arg("accela") = 1.0,
           py::arg("init") = false)
      .def("solveICMRTR", &solve_icmrtr_modern, "Solve using IC-MRTR method",
           py::arg("mat"), py::arg("rhs"), py::arg("conv_cri") = 1e-6,
           py::arg("max_ite") = 10000, py::arg("accela") = 1.0,
           py::arg("init") = false)
      .def("solveSGSMRTR", &solve_sgsmrtr_modern, "Solve using SGS-MRTR method",
           py::arg("mat"), py::arg("rhs"), py::arg("conv_cri") = 1e-6,
           py::arg("max_ite") = 10000, py::arg("init") = false)
      .def("solveICCGwithABMC", &solve_abmc_iccg_modern,
           "Solve using ABMC-ICCG method", py::arg("mat"), py::arg("rhs"),
           py::arg("conv_cri") = 1e-6, py::arg("max_ite") = 10000,
           py::arg("accela") = 1.0, py::arg("num_blocks") = 250,
           py::arg("num_colors") = 4, py::arg("init") = false)

      // Residual log access
      .def(
          "getResidualLog",
          [](MatSolvers &self) {
            std::vector<double> log;
            self.getResidualLog(log);
            return log;
          },
          "Get residual log as list");
}