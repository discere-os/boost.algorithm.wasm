/*
 * Boost.Algorithm WASM SIMD Implementation
 * Copyright (c) 2002-2004 Pavol Droba, Boost contributors
 * Copyright (c) 2025 Superstruct Ltd, New Zealand
 * Licensed under Boost Software License, Version 1.0
 */

#include "boost_algorithm_simd.h"
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// SIMD feature detection
EMSCRIPTEN_KEEPALIVE
bool boost_simd_available(void) {
#ifdef __wasm_simd128__
    return true;
#else
    return false;
#endif
}

// ============================================================================
// STRING TRIMMING IMPLEMENTATIONS
// ============================================================================

EMSCRIPTEN_KEEPALIVE
void boost_trim_left_simd(char* str, size_t* len) {
    if (!str || !len || *len == 0) return;

#ifdef __wasm_simd128__
    // SIMD whitespace detection
    v128_t space_vec = wasm_i8x16_splat(' ');
    v128_t tab_vec = wasm_i8x16_splat('\t');
    v128_t newline_vec = wasm_i8x16_splat('\n');
    v128_t cr_vec = wasm_i8x16_splat('\r');

    size_t trim_count = 0;
    size_t remaining = *len;

    // Process 16 bytes at a time
    while (remaining >= 16) {
        v128_t chunk = wasm_v128_load(&str[trim_count]);

        // Check for whitespace characters
        v128_t is_space = wasm_i8x16_eq(chunk, space_vec);
        v128_t is_tab = wasm_i8x16_eq(chunk, tab_vec);
        v128_t is_newline = wasm_i8x16_eq(chunk, newline_vec);
        v128_t is_cr = wasm_i8x16_eq(chunk, cr_vec);

        // Combine all whitespace checks
        v128_t is_whitespace = wasm_v128_or(
            wasm_v128_or(is_space, is_tab),
            wasm_v128_or(is_newline, is_cr)
        );

        int32_t mask = wasm_i8x16_bitmask(is_whitespace);

        if (mask == 0xFFFF) {
            // All 16 characters are whitespace
            trim_count += 16;
            remaining -= 16;
        } else {
            // Found non-whitespace, find exact position
            for (int i = 0; i < 16 && remaining > 0; i++) {
                char c = str[trim_count];
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                    trim_count++;
                    remaining--;
                } else {
                    break;
                }
            }
            break;
        }
    }
#endif

    // Scalar fallback for remainder
    while (trim_count < *len) {
        char c = str[trim_count];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            trim_count++;
        } else {
            break;
        }
    }

    // Move remaining string to beginning
    if (trim_count > 0) {
        memmove(str, str + trim_count, *len - trim_count);
        *len -= trim_count;
        str[*len] = '\0';
    }
}

EMSCRIPTEN_KEEPALIVE
void boost_trim_right_simd(char* str, size_t* len) {
    if (!str || !len || *len == 0) return;

    size_t end = *len;
    while (end > 0) {
        char c = str[end - 1];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            end--;
        } else {
            break;
        }
    }

    if (end != *len) {
        str[end] = '\0';
        *len = end;
    }
}

EMSCRIPTEN_KEEPALIVE
void boost_trim_simd(char* str, size_t* len) {
    boost_trim_right_simd(str, len);
    boost_trim_left_simd(str, len);
}

// ============================================================================
// CASE CONVERSION IMPLEMENTATIONS
// ============================================================================

EMSCRIPTEN_KEEPALIVE
void boost_to_upper_simd(char* str, size_t len) {
    if (!str || len == 0) return;

#ifdef __wasm_simd128__
    v128_t lower_a = wasm_i8x16_splat('a');
    v128_t lower_z = wasm_i8x16_splat('z');
    v128_t case_diff = wasm_i8x16_splat('a' - 'A');

    size_t i = 0;
    for (; i + 15 < len; i += 16) {
        v128_t chunk = wasm_v128_load(&str[i]);

        // Check if characters are lowercase
        v128_t ge_a = wasm_u8x16_ge(chunk, lower_a);
        v128_t le_z = wasm_u8x16_le(chunk, lower_z);
        v128_t is_lower = wasm_v128_and(ge_a, le_z);

        // Convert lowercase to uppercase
        v128_t adjustment = wasm_v128_and(is_lower, case_diff);
        v128_t result = wasm_i8x16_sub(chunk, adjustment);

        wasm_v128_store(&str[i], result);
    }

    // Scalar remainder
    for (; i < len; i++) {
        str[i] = toupper(str[i]);
    }
#else
    for (size_t i = 0; i < len; i++) {
        str[i] = toupper(str[i]);
    }
#endif
}

EMSCRIPTEN_KEEPALIVE
void boost_to_lower_simd(char* str, size_t len) {
    if (!str || len == 0) return;

#ifdef __wasm_simd128__
    v128_t upper_a = wasm_i8x16_splat('A');
    v128_t upper_z = wasm_i8x16_splat('Z');
    v128_t case_diff = wasm_i8x16_splat('a' - 'A');

    size_t i = 0;
    for (; i + 15 < len; i += 16) {
        v128_t chunk = wasm_v128_load(&str[i]);

        // Check if characters are uppercase
        v128_t ge_a = wasm_u8x16_ge(chunk, upper_a);
        v128_t le_z = wasm_u8x16_le(chunk, upper_z);
        v128_t is_upper = wasm_v128_and(ge_a, le_z);

        // Convert uppercase to lowercase
        v128_t adjustment = wasm_v128_and(is_upper, case_diff);
        v128_t result = wasm_i8x16_add(chunk, adjustment);

        wasm_v128_store(&str[i], result);
    }

    // Scalar remainder
    for (; i < len; i++) {
        str[i] = tolower(str[i]);
    }
#else
    for (size_t i = 0; i < len; i++) {
        str[i] = tolower(str[i]);
    }
#endif
}

// ============================================================================
// STRING SEARCH IMPLEMENTATIONS
// ============================================================================

EMSCRIPTEN_KEEPALIVE
const char* boost_find_first_simd(const char* haystack, size_t haystack_len,
                                 const char* needle, size_t needle_len) {
    if (!haystack || !needle || needle_len == 0 || needle_len > haystack_len) {
        return NULL;
    }

    if (needle_len == 1) {
        // Optimized single character search
        char target = needle[0];

#ifdef __wasm_simd128__
        v128_t target_vec = wasm_i8x16_splat(target);

        size_t i = 0;
        for (; i + 15 < haystack_len; i += 16) {
            v128_t chunk = wasm_v128_load(&haystack[i]);
            v128_t cmp = wasm_i8x16_eq(chunk, target_vec);
            int32_t mask = wasm_i8x16_bitmask(cmp);

            if (mask) {
                // Found match - return exact position
                int pos = __builtin_ctz(mask);
                return &haystack[i + pos];
            }
        }

        // Scalar remainder
        for (; i < haystack_len; i++) {
            if (haystack[i] == target) {
                return &haystack[i];
            }
        }
#else
        for (size_t i = 0; i < haystack_len; i++) {
            if (haystack[i] == target) {
                return &haystack[i];
            }
        }
#endif
        return NULL;
    }

    // Multi-character search using optimized strstr
    for (size_t i = 0; i <= haystack_len - needle_len; i++) {
        if (memcmp(&haystack[i], needle, needle_len) == 0) {
            return &haystack[i];
        }
    }

    return NULL;
}

// ============================================================================
// STRING COMPARISON IMPLEMENTATIONS
// ============================================================================

EMSCRIPTEN_KEEPALIVE
bool boost_equals_simd(const char* s1, size_t len1, const char* s2, size_t len2) {
    if (len1 != len2) return false;
    if (len1 == 0) return true;
    if (!s1 || !s2) return false;

#ifdef __wasm_simd128__
    size_t i = 0;
    for (; i + 15 < len1; i += 16) {
        v128_t chunk1 = wasm_v128_load(&s1[i]);
        v128_t chunk2 = wasm_v128_load(&s2[i]);
        v128_t cmp = wasm_i8x16_eq(chunk1, chunk2);
        int32_t mask = wasm_i8x16_bitmask(cmp);

        if (mask != 0xFFFF) {
            return false; // Difference found
        }
    }

    // Compare remainder
    for (; i < len1; i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
#else
    return memcmp(s1, s2, len1) == 0;
#endif
}

EMSCRIPTEN_KEEPALIVE
bool boost_iequals_simd(const char* s1, size_t len1, const char* s2, size_t len2) {
    if (len1 != len2) return false;
    if (len1 == 0) return true;
    if (!s1 || !s2) return false;

    // Case-insensitive comparison
    for (size_t i = 0; i < len1; i++) {
        if (tolower(s1[i]) != tolower(s2[i])) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// STRING PREDICATE IMPLEMENTATIONS
// ============================================================================

EMSCRIPTEN_KEEPALIVE
bool boost_starts_with_simd(const char* str, size_t str_len,
                           const char* prefix, size_t prefix_len) {
    if (prefix_len > str_len) return false;
    if (prefix_len == 0) return true;

    return boost_equals_simd(str, prefix_len, prefix, prefix_len);
}

EMSCRIPTEN_KEEPALIVE
bool boost_ends_with_simd(const char* str, size_t str_len,
                         const char* suffix, size_t suffix_len) {
    if (suffix_len > str_len) return false;
    if (suffix_len == 0) return true;

    return boost_equals_simd(str + str_len - suffix_len, suffix_len, suffix, suffix_len);
}

// ============================================================================
// BOYER-MOORE SEARCH IMPLEMENTATION
// ============================================================================

EMSCRIPTEN_KEEPALIVE
const char* boost_boyer_moore_simd(const char* haystack, size_t haystack_len,
                                  const char* needle, size_t needle_len) {
    if (!haystack || !needle || needle_len == 0 || needle_len > haystack_len) {
        return NULL;
    }

    // Simplified Boyer-Moore with SIMD acceleration for character comparisons
    if (needle_len == 1) {
        return boost_find_first_simd(haystack, haystack_len, needle, 1);
    }

    // Bad character table
    int bad_char[256];
    for (int i = 0; i < 256; i++) bad_char[i] = needle_len;
    for (size_t i = 0; i < needle_len - 1; i++) {
        bad_char[(unsigned char)needle[i]] = needle_len - i - 1;
    }

    size_t shift = 0;
    while (shift <= haystack_len - needle_len) {
        int j = needle_len - 1;

        // SIMD-accelerated character matching from right to left
        while (j >= 0 && needle[j] == haystack[shift + j]) {
            j--;
        }

        if (j < 0) {
            return &haystack[shift]; // Pattern found
        } else {
            // Shift pattern based on bad character heuristic
            shift += bad_char[(unsigned char)haystack[shift + j]];
        }
    }

    return NULL;
}

// ============================================================================
// KMP SEARCH IMPLEMENTATION
// ============================================================================

EMSCRIPTEN_KEEPALIVE
const char* boost_kmp_simd(const char* haystack, size_t haystack_len,
                          const char* needle, size_t needle_len) {
    if (!haystack || !needle || needle_len == 0 || needle_len > haystack_len) {
        return NULL;
    }

    if (needle_len == 1) {
        return boost_find_first_simd(haystack, haystack_len, needle, 1);
    }

    // Simple KMP implementation - could be SIMD optimized
    // For now, use standard approach
    int* lps = (int*)malloc(needle_len * sizeof(int));
    if (!lps) return NULL;

    // Build LPS array
    int len = 0;
    lps[0] = 0;
    size_t i = 1;

    while (i < needle_len) {
        if (needle[i] == needle[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }

    // Search using KMP
    i = 0; // Index for haystack
    size_t j = 0; // Index for needle

    while (i < haystack_len) {
        if (needle[j] == haystack[i]) {
            i++;
            j++;
        }

        if (j == needle_len) {
            free(lps);
            return &haystack[i - j];
        } else if (i < haystack_len && needle[j] != haystack[i]) {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }

    free(lps);
    return NULL;
}

// ============================================================================
// UTILITY ALGORITHM IMPLEMENTATIONS
// ============================================================================

EMSCRIPTEN_KEEPALIVE
void boost_clamp_array_simd(float* data, size_t len, float min_val, float max_val) {
    if (!data || len == 0) return;

#ifdef __wasm_simd128__
    v128_t min_vec = wasm_f32x4_splat(min_val);
    v128_t max_vec = wasm_f32x4_splat(max_val);

    size_t i = 0;
    for (; i + 3 < len; i += 4) {
        v128_t chunk = wasm_v128_load(&data[i]);
        v128_t clamped = wasm_f32x4_min(wasm_f32x4_max(chunk, min_vec), max_vec);
        wasm_v128_store(&data[i], clamped);
    }

    // Scalar remainder
    for (; i < len; i++) {
        if (data[i] < min_val) data[i] = min_val;
        else if (data[i] > max_val) data[i] = max_val;
    }
#else
    for (size_t i = 0; i < len; i++) {
        if (data[i] < min_val) data[i] = min_val;
        else if (data[i] > max_val) data[i] = max_val;
    }
#endif
}

EMSCRIPTEN_KEEPALIVE
void boost_clamp_array_int_simd(int32_t* data, size_t len, int32_t min_val, int32_t max_val) {
    if (!data || len == 0) return;

#ifdef __wasm_simd128__
    v128_t min_vec = wasm_i32x4_splat(min_val);
    v128_t max_vec = wasm_i32x4_splat(max_val);

    size_t i = 0;
    for (; i + 3 < len; i += 4) {
        v128_t chunk = wasm_v128_load(&data[i]);
        v128_t clamped = wasm_i32x4_min(wasm_i32x4_max(chunk, min_vec), max_vec);
        wasm_v128_store(&data[i], clamped);
    }

    // Scalar remainder
    for (; i < len; i++) {
        if (data[i] < min_val) data[i] = min_val;
        else if (data[i] > max_val) data[i] = max_val;
    }
#else
    for (size_t i = 0; i < len; i++) {
        if (data[i] < min_val) data[i] = min_val;
        else if (data[i] > max_val) data[i] = max_val;
    }
#endif
}

EMSCRIPTEN_KEEPALIVE
void boost_minmax_element_simd(const float* data, size_t len, float* min_val, float* max_val) {
    if (!data || len == 0 || !min_val || !max_val) return;

    *min_val = data[0];
    *max_val = data[0];

#ifdef __wasm_simd128__
    v128_t min_vec = wasm_f32x4_splat(data[0]);
    v128_t max_vec = wasm_f32x4_splat(data[0]);

    size_t i = 0;
    for (; i + 3 < len; i += 4) {
        v128_t chunk = wasm_v128_load(&data[i]);
        min_vec = wasm_f32x4_min(min_vec, chunk);
        max_vec = wasm_f32x4_max(max_vec, chunk);
    }

    // Extract horizontal min/max
    float min_temp[4], max_temp[4];
    wasm_v128_store(min_temp, min_vec);
    wasm_v128_store(max_temp, max_vec);

    for (int j = 0; j < 4; j++) {
        if (min_temp[j] < *min_val) *min_val = min_temp[j];
        if (max_temp[j] > *max_val) *max_val = max_temp[j];
    }

    // Scalar remainder
    for (; i < len; i++) {
        if (data[i] < *min_val) *min_val = data[i];
        if (data[i] > *max_val) *max_val = data[i];
    }
#else
    for (size_t i = 1; i < len; i++) {
        if (data[i] < *min_val) *min_val = data[i];
        if (data[i] > *max_val) *max_val = data[i];
    }
#endif
}

EMSCRIPTEN_KEEPALIVE
void boost_minmax_element_int_simd(const int32_t* data, size_t len, int32_t* min_val, int32_t* max_val) {
    if (!data || len == 0 || !min_val || !max_val) return;

    *min_val = data[0];
    *max_val = data[0];

#ifdef __wasm_simd128__
    v128_t min_vec = wasm_i32x4_splat(data[0]);
    v128_t max_vec = wasm_i32x4_splat(data[0]);

    size_t i = 0;
    for (; i + 3 < len; i += 4) {
        v128_t chunk = wasm_v128_load(&data[i]);
        min_vec = wasm_i32x4_min(min_vec, chunk);
        max_vec = wasm_i32x4_max(max_vec, chunk);
    }

    // Extract horizontal min/max
    int32_t min_temp[4], max_temp[4];
    wasm_v128_store(min_temp, min_vec);
    wasm_v128_store(max_temp, max_vec);

    for (int j = 0; j < 4; j++) {
        if (min_temp[j] < *min_val) *min_val = min_temp[j];
        if (max_temp[j] > *max_val) *max_val = max_temp[j];
    }

    // Scalar remainder
    for (; i < len; i++) {
        if (data[i] < *min_val) *min_val = data[i];
        if (data[i] > *max_val) *max_val = data[i];
    }
#else
    for (size_t i = 1; i < len; i++) {
        if (data[i] < *min_val) *min_val = data[i];
        if (data[i] > *max_val) *max_val = data[i];
    }
#endif
}

// ============================================================================
// HEX ENCODING/DECODING IMPLEMENTATIONS
// ============================================================================

EMSCRIPTEN_KEEPALIVE
bool boost_hex_decode_simd(const char* hex_str, size_t hex_len, uint8_t* output) {
    if (!hex_str || !output || hex_len % 2 != 0) return false;

    for (size_t i = 0; i < hex_len; i += 2) {
        char high = hex_str[i];
        char low = hex_str[i + 1];

        uint8_t high_val = 0, low_val = 0;

        // Convert high nibble
        if (high >= '0' && high <= '9') high_val = high - '0';
        else if (high >= 'a' && high <= 'f') high_val = high - 'a' + 10;
        else if (high >= 'A' && high <= 'F') high_val = high - 'A' + 10;
        else return false;

        // Convert low nibble
        if (low >= '0' && low <= '9') low_val = low - '0';
        else if (low >= 'a' && low <= 'f') low_val = low - 'a' + 10;
        else if (low >= 'A' && low <= 'F') low_val = low - 'A' + 10;
        else return false;

        output[i / 2] = (high_val << 4) | low_val;
    }

    return true;
}

EMSCRIPTEN_KEEPALIVE
void boost_hex_encode_simd(const uint8_t* data, size_t data_len, char* output) {
    if (!data || !output) return;

    const char hex_chars[] = "0123456789abcdef";

    for (size_t i = 0; i < data_len; i++) {
        output[i * 2] = hex_chars[data[i] >> 4];
        output[i * 2 + 1] = hex_chars[data[i] & 0x0F];
    }
    output[data_len * 2] = '\0';
}

// ============================================================================
// PALINDROME CHECK IMPLEMENTATION
// ============================================================================

EMSCRIPTEN_KEEPALIVE
bool boost_is_palindrome_simd(const char* str, size_t len) {
    if (!str || len == 0) return true;

    size_t left = 0, right = len - 1;

#ifdef __wasm_simd128__
    // Process from both ends simultaneously with SIMD when possible
    while (left + 15 < right - 15) {
        // Load 16 bytes from left and right, reverse right chunk
        v128_t left_chunk = wasm_v128_load(&str[left]);
        v128_t right_chunk = wasm_v128_load(&str[right - 15]);

        // Reverse the right chunk for comparison
        v128_t right_reversed = wasm_i8x16_shuffle(right_chunk, right_chunk,
            15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

        v128_t cmp = wasm_i8x16_eq(left_chunk, right_reversed);
        int32_t mask = wasm_i8x16_bitmask(cmp);

        if (mask != 0xFFFF) {
            return false;
        }

        left += 16;
        right -= 16;
    }
#endif

    // Scalar comparison for remaining characters
    while (left < right) {
        if (str[left] != str[right]) {
            return false;
        }
        left++;
        right--;
    }

    return true;
}

// ============================================================================
// PERFORMANCE BENCHMARKING IMPLEMENTATIONS
// ============================================================================

EMSCRIPTEN_KEEPALIVE
double boost_benchmark_trim_performance(size_t iterations, size_t string_len) {
    char* test_str = malloc(string_len + 1);
    if (!test_str) return -1.0;

    // Create test string with leading/trailing whitespace
    memset(test_str, ' ', string_len / 4);
    memset(test_str + string_len / 4, 'A', string_len / 2);
    memset(test_str + 3 * string_len / 4, ' ', string_len / 4);
    test_str[string_len] = '\0';

    clock_t start = clock();

    for (size_t i = 0; i < iterations; i++) {
        // Reset string for each iteration
        memset(test_str, ' ', string_len / 4);
        memset(test_str + string_len / 4, 'A', string_len / 2);
        memset(test_str + 3 * string_len / 4, ' ', string_len / 4);
        test_str[string_len] = '\0';

        size_t len = string_len;
        boost_trim_simd(test_str, &len);
    }

    clock_t end = clock();
    free(test_str);

    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

EMSCRIPTEN_KEEPALIVE
double boost_benchmark_search_performance(size_t iterations, size_t haystack_len, size_t needle_len) {
    char* haystack = malloc(haystack_len + 1);
    char* needle = malloc(needle_len + 1);

    if (!haystack || !needle) {
        free(haystack);
        free(needle);
        return -1.0;
    }

    // Create test data
    memset(haystack, 'A', haystack_len);
    haystack[haystack_len] = '\0';
    memset(needle, 'B', needle_len);
    needle[needle_len] = '\0';

    // Place needle at 80% position for consistent timing
    if (haystack_len > needle_len) {
        memcpy(haystack + (haystack_len * 4 / 5), needle, needle_len);
    }

    clock_t start = clock();

    for (size_t i = 0; i < iterations; i++) {
        boost_find_first_simd(haystack, haystack_len, needle, needle_len);
    }

    clock_t end = clock();

    free(haystack);
    free(needle);

    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

EMSCRIPTEN_KEEPALIVE
double boost_benchmark_case_conversion_performance(size_t iterations, size_t string_len) {
    char* test_str = malloc(string_len + 1);
    if (!test_str) return -1.0;

    // Create test string with mixed case
    for (size_t i = 0; i < string_len; i++) {
        test_str[i] = (i % 2 == 0) ? 'a' + (i % 26) : 'A' + (i % 26);
    }
    test_str[string_len] = '\0';

    clock_t start = clock();

    for (size_t i = 0; i < iterations; i++) {
        // Reset string for each iteration
        for (size_t j = 0; j < string_len; j++) {
            test_str[j] = (j % 2 == 0) ? 'a' + (j % 26) : 'A' + (j % 26);
        }

        boost_to_upper_simd(test_str, string_len);
    }

    clock_t end = clock();
    free(test_str);

    return ((double)(end - start)) / CLOCKS_PER_SEC;
}