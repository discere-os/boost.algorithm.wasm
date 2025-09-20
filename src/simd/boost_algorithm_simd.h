/*
 * Boost.Algorithm WASM SIMD Implementation
 * Copyright (c) 2002-2004 Pavol Droba, Boost contributors
 * Copyright (c) 2025 Superstruct Ltd, New Zealand
 * Licensed under Boost Software License, Version 1.0
 *
 * WASM SIMD-optimized implementations of core Boost.Algorithm functions
 * Maintains full API compatibility with existing Boost.Algorithm code
 */

#ifndef BOOST_ALGORITHM_WASM_SIMD_H
#define BOOST_ALGORITHM_WASM_SIMD_H

#include <emscripten.h>
#include <wasm_simd128.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// SIMD feature detection
EMSCRIPTEN_KEEPALIVE
bool boost_simd_available(void);

// ============================================================================
// STRING ALGORITHM OPTIMIZATIONS
// ============================================================================

// SIMD-optimized string trimming (boost::algorithm::trim)
EMSCRIPTEN_KEEPALIVE
void boost_trim_left_simd(char* str, size_t* len);

EMSCRIPTEN_KEEPALIVE
void boost_trim_right_simd(char* str, size_t* len);

EMSCRIPTEN_KEEPALIVE
void boost_trim_simd(char* str, size_t* len);

// SIMD-optimized case conversion (boost::algorithm::to_upper/to_lower)
EMSCRIPTEN_KEEPALIVE
void boost_to_upper_simd(char* str, size_t len);

EMSCRIPTEN_KEEPALIVE
void boost_to_lower_simd(char* str, size_t len);

// SIMD-optimized string search (boost::algorithm::find_first)
EMSCRIPTEN_KEEPALIVE
const char* boost_find_first_simd(const char* haystack, size_t haystack_len,
                                 const char* needle, size_t needle_len);

// SIMD-optimized string comparison (boost::algorithm::equals)
EMSCRIPTEN_KEEPALIVE
bool boost_equals_simd(const char* s1, size_t len1, const char* s2, size_t len2);

EMSCRIPTEN_KEEPALIVE
bool boost_iequals_simd(const char* s1, size_t len1, const char* s2, size_t len2);

// SIMD-optimized string predicates (boost::algorithm::starts_with/ends_with)
EMSCRIPTEN_KEEPALIVE
bool boost_starts_with_simd(const char* str, size_t str_len,
                           const char* prefix, size_t prefix_len);

EMSCRIPTEN_KEEPALIVE
bool boost_ends_with_simd(const char* str, size_t str_len,
                         const char* suffix, size_t suffix_len);

// ============================================================================
// SEARCHING ALGORITHM OPTIMIZATIONS
// ============================================================================

// SIMD-optimized Boyer-Moore search
EMSCRIPTEN_KEEPALIVE
const char* boost_boyer_moore_simd(const char* haystack, size_t haystack_len,
                                  const char* needle, size_t needle_len);

// SIMD-optimized KMP search
EMSCRIPTEN_KEEPALIVE
const char* boost_kmp_simd(const char* haystack, size_t haystack_len,
                          const char* needle, size_t needle_len);

// ============================================================================
// UTILITY ALGORITHM OPTIMIZATIONS
// ============================================================================

// SIMD-optimized clamp operations
EMSCRIPTEN_KEEPALIVE
void boost_clamp_array_simd(float* data, size_t len, float min_val, float max_val);

EMSCRIPTEN_KEEPALIVE
void boost_clamp_array_int_simd(int32_t* data, size_t len, int32_t min_val, int32_t max_val);

// SIMD-optimized minmax operations
EMSCRIPTEN_KEEPALIVE
void boost_minmax_element_simd(const float* data, size_t len, float* min_val, float* max_val);

EMSCRIPTEN_KEEPALIVE
void boost_minmax_element_int_simd(const int32_t* data, size_t len, int32_t* min_val, int32_t* max_val);

// SIMD-optimized hex operations
EMSCRIPTEN_KEEPALIVE
bool boost_hex_decode_simd(const char* hex_str, size_t hex_len, uint8_t* output);

EMSCRIPTEN_KEEPALIVE
void boost_hex_encode_simd(const uint8_t* data, size_t data_len, char* output);

// SIMD-optimized palindrome check
EMSCRIPTEN_KEEPALIVE
bool boost_is_palindrome_simd(const char* str, size_t len);

// ============================================================================
// PERFORMANCE BENCHMARKING FUNCTIONS
// ============================================================================

EMSCRIPTEN_KEEPALIVE
double boost_benchmark_trim_performance(size_t iterations, size_t string_len);

EMSCRIPTEN_KEEPALIVE
double boost_benchmark_search_performance(size_t iterations, size_t haystack_len, size_t needle_len);

EMSCRIPTEN_KEEPALIVE
double boost_benchmark_case_conversion_performance(size_t iterations, size_t string_len);

#ifdef __cplusplus
}
#endif

#endif // BOOST_ALGORITHM_WASM_SIMD_H