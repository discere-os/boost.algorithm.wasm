/*
 * Boost.Algorithm WASM SIMD API Compatibility Demo
 *
 * This demonstrates that existing Boost.Algorithm C++ code works
 * with minimal changes and gets transparent SIMD acceleration.
 */

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

// Standard Boost.Algorithm includes - these work exactly as before
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/searching.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/algorithm/minmax.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/is_palindrome.hpp>

// WASM SIMD optimizations (automatically enabled when available)
#ifdef __EMSCRIPTEN__
#include <boost/algorithm/string/wasm_simd_optimizations.hpp>
#endif

using namespace boost::algorithm;

// Performance measurement helper
class Timer {
    std::chrono::high_resolution_clock::time_point start;
public:
    Timer() : start(std::chrono::high_resolution_clock::now()) {}

    double elapsed() {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
};

int main() {
    std::cout << "=== Boost.Algorithm WASM SIMD Demo ===\n\n";

#ifdef __EMSCRIPTEN__
    // Check SIMD availability
    bool simd_enabled = boost::algorithm::detail::simd_available();
    std::cout << "WASM SIMD Support: " << (simd_enabled ? "ENABLED" : "DISABLED") << "\n\n";
#else
    std::cout << "Running in native mode (no WASM SIMD)\n\n";
#endif

    // === STRING TRIMMING DEMO ===
    std::cout << "1. STRING TRIMMING (with SIMD acceleration when available)\n";
    std::cout << "--------------------------------------------------------\n";

    std::string whitespace_str = "   \t\n  Hello, World!  \t\n  ";
    std::cout << "Original: '" << whitespace_str << "'\n";

    // These functions work exactly like standard Boost.Algorithm
    // but get SIMD acceleration automatically in WASM

    std::string left_trimmed = trim_left_copy(whitespace_str);
    std::cout << "Left trim: '" << left_trimmed << "'\n";

    std::string right_trimmed = trim_right_copy(whitespace_str);
    std::cout << "Right trim: '" << right_trimmed << "'\n";

    std::string fully_trimmed = trim_copy(whitespace_str);
    std::cout << "Full trim: '" << fully_trimmed << "'\n";

    // In-place operations also work
    std::string in_place = whitespace_str;
    trim(in_place);
    std::cout << "In-place trim: '" << in_place << "'\n\n";

    // === CASE CONVERSION DEMO ===
    std::cout << "2. CASE CONVERSION (with SIMD acceleration when available)\n";
    std::cout << "----------------------------------------------------------\n";

    std::string mixed_case = "Hello, WASM SIMD World! 123";
    std::cout << "Original: '" << mixed_case << "'\n";

    std::string upper_copy = to_upper_copy(mixed_case);
    std::cout << "Upper case: '" << upper_copy << "'\n";

    std::string lower_copy = to_lower_copy(mixed_case);
    std::cout << "Lower case: '" << lower_copy << "'\n\n";

    // === STRING SEARCH DEMO ===
    std::cout << "3. STRING SEARCH (with SIMD acceleration when available)\n";
    std::cout << "-------------------------------------------------------\n";

    std::string haystack = "The quick brown fox jumps over the lazy dog";
    std::string needle = "brown";

    std::cout << "Haystack: '" << haystack << "'\n";
    std::cout << "Needle: '" << needle << "'\n";

    auto found_it = find_first(haystack, needle);
    if (found_it != haystack.end()) {
        std::cout << "Found at position: " << std::distance(haystack.begin(), found_it) << "\n";
    } else {
        std::cout << "Not found\n";
    }

    // === STRING PREDICATES DEMO ===
    std::cout << "\n4. STRING PREDICATES (with SIMD acceleration when available)\n";
    std::cout << "-----------------------------------------------------------\n";

    std::string test_str = "Hello World";
    std::cout << "Test string: '" << test_str << "'\n";
    std::cout << "Starts with 'Hello': " << starts_with(test_str, "Hello") << "\n";
    std::cout << "Ends with 'World': " << ends_with(test_str, "World") << "\n";
    std::cout << "Equals 'Hello World': " << equals(test_str, "Hello World") << "\n";
    std::cout << "Case-insensitive equals 'HELLO WORLD': " << iequals(test_str, "HELLO WORLD") << "\n\n";

    // === ALGORITHM COMPATIBILITY DEMO ===
    std::cout << "5. OTHER BOOST ALGORITHMS (standard implementations)\n";
    std::cout << "----------------------------------------------------\n";

    // These work exactly as in standard Boost
    std::vector<int> numbers = {5, 2, 8, 1, 9, 3};
    std::cout << "Numbers: ";
    for (int n : numbers) std::cout << n << " ";
    std::cout << "\n";

    auto minmax_result = boost::minmax_element(numbers.begin(), numbers.end());
    std::cout << "Min: " << *minmax_result.first << ", Max: " << *minmax_result.second << "\n";

    // Clamp values
    for (int& n : numbers) {
        n = boost::algorithm::clamp(n, 2, 8);
    }
    std::cout << "Clamped [2,8]: ";
    for (int n : numbers) std::cout << n << " ";
    std::cout << "\n";

    // Palindrome check
    std::string palindrome = "racecar";
    std::cout << "'" << palindrome << "' is palindrome: " << boost::algorithm::is_palindrome(palindrome) << "\n";

    // Hex operations
    std::string hex_data = "48656c6c6f"; // "Hello" in hex
    std::string decoded;
    if (boost::algorithm::unhex(hex_data, std::back_inserter(decoded))) {
        std::cout << "Hex '" << hex_data << "' decodes to: '" << decoded << "'\n";
    }

    // === PERFORMANCE BENCHMARK DEMO ===
    std::cout << "\n6. PERFORMANCE BENCHMARK\n";
    std::cout << "------------------------\n";

    const size_t iterations = 10000;
    const size_t string_size = 1000;

    // Create test string with whitespace
    std::string perf_test_str(string_size / 4, ' ');
    perf_test_str += std::string(string_size / 2, 'A');
    perf_test_str += std::string(string_size / 4, ' ');

    std::cout << "Benchmarking trim operations (" << iterations << " iterations, "
              << string_size << " char strings):\n";

    Timer timer;
    for (size_t i = 0; i < iterations; ++i) {
        std::string copy = perf_test_str;
        trim(copy);
    }
    double elapsed = timer.elapsed();

    std::cout << "Time: " << elapsed << " ms\n";
    std::cout << "Throughput: " << (iterations / elapsed * 1000.0) << " operations/second\n";
    std::cout << "Bandwidth: " << (iterations * string_size / elapsed / 1000.0) << " MB/second\n";

#ifdef __EMSCRIPTEN__
    if (simd_enabled) {
        std::cout << "Note: Performance includes WASM SIMD acceleration!\n";
    }
#endif

    std::cout << "\n=== Demo Complete ===\n";
    std::cout << "This demonstrates that existing Boost.Algorithm code works\n";
    std::cout << "with minimal changes while getting transparent SIMD acceleration!\n";

    return 0;
}