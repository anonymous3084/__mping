// This file is part of xxxxx-1.
//
// Copyright 2022 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version. xxxxx-1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with xxxxx-1.  If not, see
// <https://www.gnu.org/licenses/>.

/// @file
/// @brief Redefines the KASSERT macro such that assertions throw exceptions instead of aborting the process.
/// This is needed because GoogleTest does not support death tests in a multithreaded program (MPI spawns multiple
/// threads).
///
/// *NOTE THAT THIS HEADER MUST BE INCLUDED BEFORE ANY OTHER xxxxx-1 HEADERS* since it redefines the KASSERT macro.
/// This must happen before the preprocessor substitutes the macro invocations.
#pragma once

#if defined(KASSERT) || defined(EXPECT_KASSERT_FAILS) || defined(ASSERT_KASSERT_FAILS) || defined(xxxxx-1_NOEXCEPT)
    #error "Bad #include order: this header must be included first"
#endif

#include <exception>
#include <string>

//
// Disable noexcept
//
#include "xxxxx-1ing/noexcept.hpp"

#undef xxxxx-1_NOEXCEPT
#define xxxxx-1_NOEXCEPT
#undef xxxxx-1_CONDITIONAL_NOEXCEPT
#define xxxxx-1_CONDITIONAL_NOEXCEPT(condition)

//
// Redefine KASSERT()
//

#include <kassert/kassert.hpp>

#include "xxxxx-1ing/assertion_levels.hpp"

// Redefine KASSERT implementation to throw an exception
#undef KASSERT_KASSERT_HPP_KASSERT_IMPL
#define KASSERT_KASSERT_HPP_KASSERT_IMPL(type, expression, message, level)                                       \
    do {                                                                                                         \
        if constexpr (kassert::internal::assertion_enabled(level)) {                                             \
            if (!(expression)) {                                                                                 \
                throw xxxxx-1ing::testing::KassertTestingException(                                                 \
                    (kassert::internal::RrefOStringstreamLogger{std::ostringstream{}} << message).stream().str() \
                );                                                                                               \
            }                                                                                                    \
        }                                                                                                        \
    } while (false)

// Makros to test for failed KASSERTs
// EXPECT that any KASSERT assertion failed. The failure message is ignored since EXPECT_THROW does not support one.
#define EXPECT_KASSERT_FAILS(code, failure_message) \
    EXPECT_THROW({ code; }, ::xxxxx-1ing::testing::KassertTestingException);

// ASSERT that any KASSERT assertion failed. The failure message is ignored since ASSERT_THROW does not support one.
#define ASSERT_KASSERT_FAILS(code, failure_message) \
    ASSERT_THROW({ code; }, ::xxxxx-1ing::testing::KassertTestingException);

// Dummy exception class used for remapping assertions to throwing exceptions.
namespace xxxxx-1ing::testing {
class KassertTestingException : public std::exception {
public:
    // Assertion message (no expression decomposition)
    KassertTestingException(std::string message) : _message(std::move(message)) {}

    char const* what() const noexcept override {
        return _message.c_str();
    }

private:
    std::string _message;
};
} // namespace xxxxx-1ing::testing
