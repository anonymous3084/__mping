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
/// @brief Defines the macro `xxxxx-1_NOEXCEPT` to be used instad of `noexcept`.
#pragma once

/// @brief `noexcept` macro.
#define xxxxx-1_NOEXCEPT noexcept

/// @brief Conditional noexcept `noexcept(...)` macro.
#define xxxxx-1_CONDITIONAL_NOEXCEPT(condition) noexcept(condition)
