from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
import os


class SparseSolvConan(ConanFile):
    name = "sparse-solv"
    version = "0.1.0"
    package_type = "library"

    # Package metadata
    license = "MIT"
    author = "SparseSolv Contributors"
    url = "https://github.com/JP-MARs/SparseSolv"
    description = "High-performance sparse linear algebra library with iterative solvers (ICCG, MRTR, SGS)"
    topics = ("sparse-matrix", "linear-algebra", "iterative-solvers", "hpc", "scientific-computing")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False], 
        "fPIC": [True, False],
        "with_python_bindings": [True, False],
        "with_mkl": [True, False],
        "with_openmp": [True, False]
    }
    default_options = {
        "shared": False, 
        "fPIC": True,
        "with_python_bindings": False,
        "with_mkl": False,
        "with_openmp": True
    }

    # Sources are located in the same place as this recipe
    exports_sources = "CMakeLists.txt", "src/*", "include/*", "LICENSE", "README.md"

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def requirements(self):
        # Core dependencies
        self.requires("eigen/3.4.0")
        
        # Optional dependencies based on options
        if self.options.with_python_bindings:
            self.requires("pybind11/2.13.6")

    def system_requirements(self):
        if self.options.with_mkl:
            # MKL is typically a system dependency or handled via compiler flags
            self.output.info("Intel MKL support enabled - ensure MKL is available in the system")

    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        
        tc = CMakeToolchain(self)
        # Pass options to CMake
        tc.variables["SPARSE_SOLV_WITH_PYTHON_BINDINGS"] = self.options.with_python_bindings
        tc.variables["SPARSE_SOLV_WITH_MKL"] = self.options.with_mkl
        tc.variables["SPARSE_SOLV_WITH_OPENMP"] = self.options.with_openmp
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        # Copy license
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        # Core library
        self.cpp_info.components["sparse-solv"].libs = ["sparse-solv"]
        self.cpp_info.components["sparse-solv"].requires = ["eigen::eigen"]
        
        # Add OpenMP if enabled
        if self.options.with_openmp:
            if self.settings.os == "Linux":
                self.cpp_info.components["sparse-solv"].system_libs.append("gomp")
            elif self.settings.os == "Macos":
                self.cpp_info.components["sparse-solv"].system_libs.append("omp")
            # Windows OpenMP is typically handled by the compiler
        
        # Add MKL if enabled
        if self.options.with_mkl:
            if self.settings.compiler == "intel-cc":
                self.cpp_info.components["sparse-solv"].sharedlinkflags.append("-qmkl")
                self.cpp_info.components["sparse-solv"].exelinkflags.append("-qmkl")
        
        # Python bindings component (if enabled)
        if self.options.with_python_bindings:
            self.cpp_info.components["python-bindings"].libs = ["SparseSolvPy"]
            self.cpp_info.components["python-bindings"].requires = ["sparse-solv", "pybind11::pybind11"]
        
        # Set package-level info
        self.cpp_info.libs = []  # Clear this since we use components
        self.cpp_info.set_property("cmake_file_name", "SparseSolv")
        self.cpp_info.set_property("cmake_target_name", "SparseSolv::sparse-solv")

