// This file is part of xxxxx-1.
//
// Copyright 2022-2024 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version. xxxxx-1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with xxxxx-1.  If not, see
// <https://www.gnu.org/licenses/>.

#pragma once

#include <mpi.h>

#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/error_handling.hpp"
/// @addtogroup xxxxx-1ing_collectives
/// @{

/// @brief Perform a \c MPI_Barrier on this communicator.
///
/// Barrier takes no parameters. Any parameters passed will cause a compilation error.
///
/// The parameter pack prohibits the compiler from compiling this function when it's not used.
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename... Args>
void xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::barrier(Args... args) const {
    using namespace xxxxx-1ing::internal;
    static_assert(sizeof...(args) == 0, "You may not pass any arguments to barrier().");

    [[maybe_unused]] int err = MPI_Barrier(mpi_communicator());
    this->mpi_error_hook(err, "MPI_Barrier");
}
/// @}
