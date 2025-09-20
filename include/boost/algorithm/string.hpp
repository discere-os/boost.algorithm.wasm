//  Boost string_algo library string_algo.hpp header file  ---------------------------//

//  Copyright Pavol Droba 2002-2004.
//  Copyright (c) 2025 Superstruct Ltd, New Zealand (WASM SIMD enhancements)
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/ for updates, documentation, and revision history.

#ifndef BOOST_STRING_ALGO_HPP
#define BOOST_STRING_ALGO_HPP

/*! \file
    Cumulative include for string_algo library - WASM SIMD Enhanced Version
    Provides full API compatibility with existing Boost.Algorithm string operations
    with transparent WASM SIMD acceleration when available.
*/

// WASM SIMD optimization detection and configuration
#ifdef __EMSCRIPTEN__
#include "../../../src/simd/boost_algorithm_simd.h"

namespace boost {
namespace algorithm {
namespace detail {

// Runtime detection of SIMD availability
inline bool simd_available() {
    static bool checked = false;
    static bool available = false;

    if (!checked) {
        available = boost_simd_available();
        checked = true;
    }

    return available;
}

} // namespace detail
} // namespace algorithm
} // namespace boost

#endif // __EMSCRIPTEN__

#include <boost/algorithm/string/std_containers_traits.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/find_iterator.hpp>


#endif  // BOOST_STRING_ALGO_HPP
