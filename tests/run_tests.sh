#!/bin/bash

# Comprehensive test runner for SparseSolv
echo "======================================="
echo "SparseSolv Comprehensive Test Suite"
echo "======================================="

# Set up environment
export BUILD_DIR="../build/Release"
export TEST_DIR="$(pwd)"
export RESULTS_DIR="$TEST_DIR/results"

# Create results directory
mkdir -p "$RESULTS_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to run a test and record results
run_test() {
    local test_name="$1"
    local test_command="$2"
    local log_file="$RESULTS_DIR/${test_name}.log"
    
    echo -n "Running $test_name... "
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if eval "$test_command" > "$log_file" 2>&1; then
        echo -e "${GREEN}PASSED${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        return 0
    else
        echo -e "${RED}FAILED${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo "  Log: $log_file"
        return 1
    fi
}

# Check if we can find the build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Warning: Build directory not found at $BUILD_DIR${NC}"
    echo "Please build the project first using the commands in CLAUDE.md"
    echo ""
fi

# Test 1: Build the test suite
echo "Building test suite..."
if [ -f "../CMakeLists.txt" ]; then
    run_test "test_build" "cd .. && mkdir -p build_tests && cd build_tests && cmake ../tests && make"
    
    if [ $? -eq 0 ]; then
        TEST_BUILD_DIR="../build_tests"
        
        # Run C++ tests
        echo ""
        echo "Running C++ Tests:"
        echo "=================="
        
        run_test "sparse_mat_test" "$TEST_BUILD_DIR/test_sparse_mat"
        run_test "sparse_mat_c_test" "$TEST_BUILD_DIR/test_sparse_mat_c"
        run_test "mat_solvers_test" "$TEST_BUILD_DIR/test_mat_solvers"
        run_test "api_test" "$TEST_BUILD_DIR/test_api"
        run_test "integration_test" "$TEST_BUILD_DIR/integration_test"
    fi
else
    echo -e "${YELLOW}Warning: CMakeLists.txt not found. Skipping build tests.${NC}"
fi

# Test 2: Run existing test_package
echo ""
echo "Running existing test package:"
echo "=============================="

if [ -f "../test_package/build/gcc-12-x86_64-gnu17-release/example" ]; then
    run_test "existing_test_package" "../test_package/build/gcc-12-x86_64-gnu17-release/example"
else
    echo -e "${YELLOW}Existing test package not found. Run conan create . --build=missing first.${NC}"
fi

# Test 3: Python bindings tests
echo ""
echo "Running Python Tests:"
echo "===================="

# Check if Python bindings are available
python3 -c "import SparseSolvPy" 2>/dev/null
if [ $? -eq 0 ]; then
    run_test "python_bindings_test" "cd $TEST_DIR && python3 test_python_bindings.py"
else
    echo -e "${YELLOW}Python bindings not available. Build with SPARSE_SOLV_WITH_PYTHON_BINDINGS=True to enable.${NC}"
fi

# Test 4: Memory leak checks (if valgrind is available)
echo ""
echo "Memory Tests:"
echo "============="

if command -v valgrind >/dev/null 2>&1; then
    if [ -f "$TEST_BUILD_DIR/test_sparse_mat" ]; then
        run_test "memory_leak_test" "valgrind --leak-check=full --error-exitcode=1 $TEST_BUILD_DIR/test_sparse_mat"
    fi
else
    echo -e "${YELLOW}Valgrind not available. Skipping memory leak tests.${NC}"
fi

# Test 5: Performance benchmarks
echo ""
echo "Performance Tests:"
echo "=================="

if [ -f "$TEST_BUILD_DIR/integration_test" ]; then
    run_test "performance_test" "timeout 60s $TEST_BUILD_DIR/integration_test"
else
    echo -e "${YELLOW}Integration test not available. Skipping performance tests.${NC}"
fi

# Test 6: Example programs
echo ""
echo "Example Tests:"
echo "=============="

# VoxelFEM example (if data is available)
if [ -f "../examples/VoxelFEM/main.cpp" ]; then
    echo -e "${YELLOW}VoxelFEM example found but requires mesh data. Skipping.${NC}"
fi

# Python example
if [ -f "../examples/Pybind_example.py" ]; then
    python3 -c "import SparseSolvPy" 2>/dev/null
    if [ $? -eq 0 ]; then
        run_test "python_example" "cd ../examples && python3 Pybind_example.py"
    else
        echo -e "${YELLOW}Python bindings not available for example test.${NC}"
    fi
fi

# Final results
echo ""
echo "======================================="
echo "Test Results Summary"
echo "======================================="
echo "Total tests: $TOTAL_TESTS"
echo -e "Passed: ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed: ${RED}$FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. Check logs in $RESULTS_DIR${NC}"
    exit 1
fi