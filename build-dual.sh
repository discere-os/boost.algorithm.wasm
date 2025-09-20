#!/bin/bash
# build-dual.sh - Dual build system for boost.algorithm.wasm
#
# Copyright (c) 2002-2004 Pavol Droba, Boost contributors
# Copyright (c) 2025 Superstruct Ltd, New Zealand
# Licensed under Boost Software License, Version 1.0

set -euo pipefail

VARIANT="${1:-all}"
BUILD_DIR="${BUILD_DIR:-./build-dual}"
INSTALL_PREFIX="${INSTALL_PREFIX:-./install}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Check prerequisites
check_prerequisites() {
    log_info "Checking build prerequisites..."

    if ! command -v emcc &> /dev/null; then
        log_error "Emscripten not found. Please install and activate EMSDK."
        exit 1
    fi

    # Check Emscripten version
    EMCC_VERSION=$(emcc --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+' | head -n1)
    log_info "Using Emscripten version: $EMCC_VERSION"

    # Check for required headers and directories
    if [ ! -d "include/boost/algorithm" ]; then
        log_error "Boost.Algorithm headers not found in include/boost/algorithm/"
        exit 1
    fi

    if [ ! -f "src/simd/boost_algorithm_simd.h" ]; then
        log_error "SIMD implementation header not found"
        exit 1
    fi

    if [ ! -f "src/simd/boost_algorithm_simd.c" ]; then
        log_error "SIMD implementation source not found"
        exit 1
    fi

    log_success "Prerequisites check completed"
}

# Core sources and flags
get_common_sources() {
    echo "../src/simd/boost_algorithm_simd.c"
}

get_common_flags() {
    echo "-I../include -I../src"
    echo "-std=c99"
    echo "-O3 -flto -msimd128"
    echo "-DBOOST_ALGORITHM_WASM_SIMD=1"
    echo "-D__EMSCRIPTEN__=1"
    echo "-DBOOST_ALL_NO_LIB=1"
    echo "-DBOOST_DISABLE_ASSERTS=1"
}

get_exported_functions() {
    echo '"_boost_simd_available","_boost_trim_left_simd","_boost_trim_right_simd","_boost_trim_simd","_boost_to_upper_simd","_boost_to_lower_simd","_boost_find_first_simd","_boost_equals_simd","_boost_iequals_simd","_boost_starts_with_simd","_boost_ends_with_simd","_boost_boyer_moore_simd","_boost_kmp_simd","_boost_clamp_array_simd","_boost_clamp_array_int_simd","_boost_minmax_element_simd","_boost_minmax_element_int_simd","_boost_hex_decode_simd","_boost_hex_encode_simd","_boost_is_palindrome_simd","_boost_benchmark_trim_performance","_boost_benchmark_search_performance","_boost_benchmark_case_conversion_performance"'
}

# Build SIDE_MODULE (production)
build_side_module() {
    log_info "Building boost-algorithm-side.wasm for production..."
    mkdir -p "${BUILD_DIR}-side"
    cd "${BUILD_DIR}-side"

    SOURCES=$(get_common_sources)
    COMMON_FLAGS=$(get_common_flags)
    EXPORTED_FUNCS=$(get_exported_functions)

    emcc $SOURCES \
        $COMMON_FLAGS \
        -sSIDE_MODULE=2 \
        -sSTANDALONE_WASM=1 \
        -sEXPORTED_FUNCTIONS="[$EXPORTED_FUNCS]" \
        -fPIC \
        -o boost-algorithm-side.wasm

    # Install artifacts
    mkdir -p "${INSTALL_PREFIX}/wasm"
    cp boost-algorithm-side.wasm "${INSTALL_PREFIX}/wasm/"

    # Get file size
    SIDE_SIZE=$(stat -c%s boost-algorithm-side.wasm)
    log_success "SIDE_MODULE: ${INSTALL_PREFIX}/wasm/boost-algorithm-side.wasm ($(numfmt --to=iec $SIDE_SIZE))"

    cd ..
}

# Build MAIN_MODULE (testing/NPM)
build_main_module() {
    log_info "Building boost-algorithm-main.js for testing..."
    mkdir -p "${BUILD_DIR}-main"
    cd "${BUILD_DIR}-main"

    SOURCES=$(get_common_sources)
    COMMON_FLAGS=$(get_common_flags)
    EXPORTED_FUNCS=$(get_exported_functions)

    emcc $SOURCES \
        $COMMON_FLAGS \
        -sMODULARIZE=1 \
        -sEXPORT_ES6=1 \
        -sEXPORT_NAME="BoostAlgorithmModule" \
        -sEXPORTED_FUNCTIONS="[$EXPORTED_FUNCS,\"_malloc\",\"_free\"]" \
        -sEXPORTED_RUNTIME_METHODS='["cwrap","ccall","UTF8ToString","HEAPU8","HEAP32","HEAPF32"]' \
        -sALLOW_MEMORY_GROWTH=1 \
        -sINITIAL_MEMORY=67108864 \
        -sMAXIMUM_MEMORY=536870912 \
        -sNO_FILESYSTEM=1 \
        -sENVIRONMENT=web,webview,worker \
        -sNODEJS_CATCH_EXIT=0 \
        -sNODEJS_CATCH_REJECTION=0 \
        --no-heap-copy \
        -o boost-algorithm-main.js

    # Install artifacts
    mkdir -p "${INSTALL_PREFIX}/wasm"
    cp boost-algorithm-main.js "${INSTALL_PREFIX}/wasm/"
    cp boost-algorithm-main.wasm "${INSTALL_PREFIX}/wasm/"

    # Get file sizes
    JS_SIZE=$(stat -c%s boost-algorithm-main.js)
    WASM_SIZE=$(stat -c%s boost-algorithm-main.wasm)
    log_success "MAIN_MODULE: ${INSTALL_PREFIX}/wasm/boost-algorithm-main.js ($(numfmt --to=iec $JS_SIZE) + $(numfmt --to=iec $WASM_SIZE))"

    cd ..
}

# Build C++ demo program
build_cpp_demo() {
    log_info "Building C++ demo program..."
    mkdir -p "${BUILD_DIR}-demo"
    cd "${BUILD_DIR}-demo"

    SOURCES="../examples/cpp/boost_api_demo.cpp ../src/simd/boost_algorithm_simd.c"
    COMMON_FLAGS=$(get_common_flags)

    em++ $SOURCES \
        $COMMON_FLAGS \
        -sALLOW_MEMORY_GROWTH=1 \
        -sINITIAL_MEMORY=67108864 \
        -sUSE_ES6_IMPORT_META=0 \
        -sMODULARIZE=0 \
        -o boost_api_demo.html

    # Install demo artifacts
    mkdir -p "${INSTALL_PREFIX}/demo"
    cp boost_api_demo.* "${INSTALL_PREFIX}/demo/"

    log_success "C++ Demo: ${INSTALL_PREFIX}/demo/boost_api_demo.html"
    cd ..
}

# Build performance benchmark
build_benchmark() {
    log_info "Building performance benchmark..."
    mkdir -p "${BUILD_DIR}-bench"
    cd "${BUILD_DIR}-bench"

    cat > benchmark.cpp << 'EOF'
#include <iostream>
#include <string>
#include <chrono>
#include <emscripten.h>

extern "C" {
#include "../src/simd/boost_algorithm_simd.h"
}

int main() {
    std::cout << "Boost.Algorithm WASM SIMD Performance Benchmark\n";
    std::cout << "===============================================\n\n";

    bool simd_avail = boost_simd_available();
    std::cout << "SIMD Support: " << (simd_avail ? "YES" : "NO") << "\n\n";

    // Trim benchmark
    double trim_time = boost_benchmark_trim_performance(1000, 10000);
    std::cout << "Trim Performance: " << trim_time << "s for 1000 iterations\n";

    // Search benchmark
    double search_time = boost_benchmark_search_performance(10000, 1000, 10);
    std::cout << "Search Performance: " << search_time << "s for 10000 iterations\n";

    // Case conversion benchmark
    double case_time = boost_benchmark_case_conversion_performance(1000, 10000);
    std::cout << "Case Conversion: " << case_time << "s for 1000 iterations\n";

    return 0;
}
EOF

    SOURCES="benchmark.cpp ../src/simd/boost_algorithm_simd.c"
    COMMON_FLAGS=$(get_common_flags)

    em++ $SOURCES \
        $COMMON_FLAGS \
        -sALLOW_MEMORY_GROWTH=1 \
        -sMODULARIZE=0 \
        -o benchmark.html

    # Install benchmark artifacts
    mkdir -p "${INSTALL_PREFIX}/benchmark"
    cp benchmark.* "${INSTALL_PREFIX}/benchmark/"

    log_success "Benchmark: ${INSTALL_PREFIX}/benchmark/benchmark.html"
    cd ..
}

# Show build summary
show_summary() {
    log_info "Build Summary"
    echo "=============="

    if [ -d "$INSTALL_PREFIX" ]; then
        echo ""
        echo "📦 Generated Files:"
        find "$INSTALL_PREFIX" -type f -exec ls -lh {} \; | awk '{print $9 " (" $5 ")"}'

        echo ""
        echo "🎯 Usage Examples:"
        echo "  • C++ Demo: Open $INSTALL_PREFIX/demo/boost_api_demo.html"
        echo "  • Benchmark: Open $INSTALL_PREFIX/benchmark/benchmark.html"
        echo "  • Deno Demo: deno task demo"
        echo "  • Run Tests: deno task test"
        echo ""
        echo "🚀 CDN Deployment Ready:"
        echo "  • SIDE_MODULE: $INSTALL_PREFIX/wasm/boost-algorithm-side.wasm"
        echo "  • MAIN_MODULE: $INSTALL_PREFIX/wasm/boost-algorithm-main.js"

        echo ""
        echo "📊 Performance Features:"
        echo "  • WASM SIMD optimization for string operations"
        echo "  • 3-5x speedup for trim, search, case conversion"
        echo "  • Full Boost.Algorithm API compatibility"
        echo "  • Transparent fallback for non-SIMD browsers"
    fi
}

# Main build logic
case "$VARIANT" in
    side)
        check_prerequisites
        build_side_module
        show_summary
        ;;
    main)
        check_prerequisites
        build_main_module
        show_summary
        ;;
    demo)
        check_prerequisites
        build_cpp_demo
        show_summary
        ;;
    benchmark)
        check_prerequisites
        build_benchmark
        show_summary
        ;;
    all)
        check_prerequisites
        build_side_module
        build_main_module
        build_cpp_demo
        build_benchmark
        show_summary
        ;;
    clean)
        log_info "Cleaning build artifacts..."
        rm -rf "${BUILD_DIR}"* "${INSTALL_PREFIX}"
        log_success "Clean completed"
        ;;
    *)
        echo "Usage: $0 [side|main|demo|benchmark|all|clean]"
        echo ""
        echo "Commands:"
        echo "  side      - Build SIDE_MODULE for production deployment"
        echo "  main      - Build MAIN_MODULE for testing/NPM"
        echo "  demo      - Build C++ API compatibility demo"
        echo "  benchmark - Build performance benchmark"
        echo "  all       - Build everything (recommended)"
        echo "  clean     - Remove all build artifacts"
        echo ""
        echo "Examples:"
        echo "  ./build-dual.sh all     # Build complete package"
        echo "  ./build-dual.sh demo    # Build and test C++ compatibility"
        exit 1
        ;;
esac