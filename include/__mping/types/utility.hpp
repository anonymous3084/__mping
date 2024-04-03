// This file is part of xxxxx-1.
//
// Copyright 2024 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version. xxxxx-1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with xxxxx-1.  If not, see
// <https://www.gnu.org/licenses/>.
//
#pragma once

#include <utility>

#include <xxxxx-1ing/mpi_datatype.hpp>

namespace xxxxx-1ing {
/// @brief Specialization of the `mpi_type_traits` type trait for `std::pair`, which represents the pair as MPI struct
/// type.
/// @note Using struct types may have performance pitfalls if the types has padding. For maximum performance, use the
/// unsafe version in \ref xxxxx-1ing/types/unsafe/utility.hpp.
template <typename First, typename Second>
struct mpi_type_traits<
    std::pair<First, Second>,
    std::enable_if_t<has_static_type_v<First> && has_static_type_v<Second>>> : struct_type<std::pair<First, Second>> {};
} // namespace xxxxx-1ing
