//  Boost Algorithm WASM SIMD Optimizations  ---------------------------//
//
//  Copyright (c) 2025 Superstruct Ltd, New Zealand
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_ALGORITHM_STRING_WASM_SIMD_OPTIMIZATIONS_HPP
#define BOOST_ALGORITHM_STRING_WASM_SIMD_OPTIMIZATIONS_HPP

#include <boost/algorithm/string/config.hpp>
#include <string>
#include <type_traits>

#ifdef __EMSCRIPTEN__
#include "../../../../src/simd/boost_algorithm_simd.h"

namespace boost {
namespace algorithm {
namespace detail {

// WASM SIMD specialization for std::string trimming
template<>
inline void trim_left_if_impl<std::string>(std::string& Input) {
    if (simd_available() && !Input.empty()) {
        // Use WASM SIMD optimized version for std::string
        char* data = const_cast<char*>(Input.data());
        size_t len = Input.length();

        boost_trim_left_simd(data, &len);

        if (len != Input.length()) {
            Input.resize(len);
        }
    } else {
        // Fallback to standard implementation
        Input.erase(Input.begin(), std::find_if_not(Input.begin(), Input.end(), [](char c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        }));
    }
}

template<>
inline void trim_right_if_impl<std::string>(std::string& Input) {
    if (simd_available() && !Input.empty()) {
        // Use WASM SIMD optimized version for std::string
        char* data = const_cast<char*>(Input.data());
        size_t len = Input.length();

        boost_trim_right_simd(data, &len);

        if (len != Input.length()) {
            Input.resize(len);
        }
    } else {
        // Fallback to standard implementation
        Input.erase(std::find_if_not(Input.rbegin(), Input.rend(), [](char c) {
            return c == ' ' || c == '\t' || c == '\n' || c == '\r';
        }).base(), Input.end());
    }
}

template<>
inline void trim_if_impl<std::string>(std::string& Input) {
    if (simd_available() && !Input.empty()) {
        // Use WASM SIMD optimized version for std::string
        char* data = const_cast<char*>(Input.data());
        size_t len = Input.length();

        boost_trim_simd(data, &len);

        if (len != Input.length()) {
            Input.resize(len);
        }
    } else {
        // Fallback to standard implementation
        trim_right_if_impl<std::string>(Input);
        trim_left_if_impl<std::string>(Input);
    }
}

// WASM SIMD specialization for case conversion
template<>
inline void to_upper_impl<std::string>(std::string& Input) {
    if (simd_available() && !Input.empty()) {
        // Use WASM SIMD optimized version
        boost_to_upper_simd(const_cast<char*>(Input.data()), Input.length());
    } else {
        // Fallback to standard implementation
        std::transform(Input.begin(), Input.end(), Input.begin(), ::toupper);
    }
}

template<>
inline void to_lower_impl<std::string>(std::string& Input) {
    if (simd_available() && !Input.empty()) {
        // Use WASM SIMD optimized version
        boost_to_lower_simd(const_cast<char*>(Input.data()), Input.length());
    } else {
        // Fallback to standard implementation
        std::transform(Input.begin(), Input.end(), Input.begin(), ::tolower);
    }
}

// WASM SIMD specialization for string search
template<>
inline std::string::const_iterator find_first_impl<std::string>(
    const std::string& Input,
    const std::string& Search) {

    if (simd_available() && !Input.empty() && !Search.empty()) {
        // Use WASM SIMD optimized version
        const char* result = boost_find_first_simd(
            Input.data(), Input.length(),
            Search.data(), Search.length()
        );

        if (result) {
            return Input.begin() + (result - Input.data());
        } else {
            return Input.end();
        }
    } else {
        // Fallback to standard implementation
        auto pos = Input.find(Search);
        return (pos == std::string::npos) ? Input.end() : Input.begin() + pos;
    }
}

// WASM SIMD specialization for string comparison
template<>
inline bool equals_impl<std::string>(const std::string& Input1, const std::string& Input2) {
    if (simd_available()) {
        // Use WASM SIMD optimized version
        return boost_equals_simd(
            Input1.data(), Input1.length(),
            Input2.data(), Input2.length()
        );
    } else {
        // Fallback to standard implementation
        return Input1 == Input2;
    }
}

template<>
inline bool iequals_impl<std::string>(const std::string& Input1, const std::string& Input2) {
    if (simd_available()) {
        // Use WASM SIMD optimized version
        return boost_iequals_simd(
            Input1.data(), Input1.length(),
            Input2.data(), Input2.length()
        );
    } else {
        // Fallback to standard implementation
        if (Input1.length() != Input2.length()) return false;
        return std::equal(Input1.begin(), Input1.end(), Input2.begin(),
            [](char a, char b) {
                return std::tolower(a) == std::tolower(b);
            });
    }
}

// WASM SIMD specialization for predicates
template<>
inline bool starts_with_impl<std::string>(const std::string& Input, const std::string& Test) {
    if (simd_available()) {
        // Use WASM SIMD optimized version
        return boost_starts_with_simd(
            Input.data(), Input.length(),
            Test.data(), Test.length()
        );
    } else {
        // Fallback to standard implementation
        if (Test.length() > Input.length()) return false;
        return std::equal(Test.begin(), Test.end(), Input.begin());
    }
}

template<>
inline bool ends_with_impl<std::string>(const std::string& Input, const std::string& Test) {
    if (simd_available()) {
        // Use WASM SIMD optimized version
        return boost_ends_with_simd(
            Input.data(), Input.length(),
            Test.data(), Test.length()
        );
    } else {
        // Fallback to standard implementation
        if (Test.length() > Input.length()) return false;
        return std::equal(Test.rbegin(), Test.rend(), Input.rbegin());
    }
}

} // namespace detail

// High-level API wrappers that automatically use SIMD when available

// SIMD-accelerated trim functions for std::string
inline void trim_left(std::string& Input) {
    detail::trim_left_if_impl<std::string>(Input);
}

inline void trim_right(std::string& Input) {
    detail::trim_right_if_impl<std::string>(Input);
}

inline void trim(std::string& Input) {
    detail::trim_if_impl<std::string>(Input);
}

inline std::string trim_left_copy(const std::string& Input) {
    std::string result = Input;
    trim_left(result);
    return result;
}

inline std::string trim_right_copy(const std::string& Input) {
    std::string result = Input;
    trim_right(result);
    return result;
}

inline std::string trim_copy(const std::string& Input) {
    std::string result = Input;
    trim(result);
    return result;
}

// SIMD-accelerated case conversion for std::string
inline void to_upper(std::string& Input) {
    detail::to_upper_impl<std::string>(Input);
}

inline void to_lower(std::string& Input) {
    detail::to_lower_impl<std::string>(Input);
}

inline std::string to_upper_copy(const std::string& Input) {
    std::string result = Input;
    to_upper(result);
    return result;
}

inline std::string to_lower_copy(const std::string& Input) {
    std::string result = Input;
    to_lower(result);
    return result;
}

// SIMD-accelerated search functions for std::string
inline std::string::const_iterator find_first(const std::string& Input, const std::string& Search) {
    return detail::find_first_impl<std::string>(Input, Search);
}

// SIMD-accelerated comparison functions for std::string
inline bool equals(const std::string& Input1, const std::string& Input2) {
    return detail::equals_impl<std::string>(Input1, Input2);
}

inline bool iequals(const std::string& Input1, const std::string& Input2) {
    return detail::iequals_impl<std::string>(Input1, Input2);
}

// SIMD-accelerated predicate functions for std::string
inline bool starts_with(const std::string& Input, const std::string& Test) {
    return detail::starts_with_impl<std::string>(Input, Test);
}

inline bool ends_with(const std::string& Input, const std::string& Test) {
    return detail::ends_with_impl<std::string>(Input, Test);
}

} // namespace algorithm
} // namespace boost

#endif // __EMSCRIPTEN__

#endif // BOOST_ALGORITHM_STRING_WASM_SIMD_OPTIMIZATIONS_HPP