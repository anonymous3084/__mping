// This file is part of xxxxx-1.
//
// Copyright 2023 The xxxxx-1 Authors
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
#include "xxxxx-1ing/named_parameter_check.hpp"
#include "xxxxx-1ing/named_parameter_selection.hpp"
#include "xxxxx-1ing/named_parameter_types.hpp"
#include "xxxxx-1ing/named_parameters.hpp"
#include "xxxxx-1ing/parameter_objects.hpp"
#include "xxxxx-1ing/request.hpp"
#include "xxxxx-1ing/result.hpp"

/// @addtogroup xxxxx-1ing_collectives
/// @{

/// @brief Perform a non-blocking barrier synchronization on this communicator using \c MPI_Ibarrier. The call is
/// associated with a \ref xxxxx-1ing::Request (either allocated by xxxxx-1 or provided by the user). Only when the request
/// has completed, it is guaranteed that all ranks have reached the barrier.
///
/// The following parameters are optional:
/// - \ref xxxxx-1ing::request() The request object to associate this operation with. Defaults to a library allocated
/// request object, which can be accessed via the returned result.
///
/// @tparam Args Automatically deduced template parameters.
/// @param args All required and any number of the optional buffers described above.
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename... Args>
auto xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::ibarrier(Args... args) const {
    using namespace xxxxx-1ing::internal;
    xxxxx-1_CHECK_PARAMETERS(Args, xxxxx-1_REQUIRED_PARAMETERS(), xxxxx-1_OPTIONAL_PARAMETERS(request));

    using default_request_param = decltype(xxxxx-1ing::request());
    auto&& request_param =
        internal::select_parameter_type_or_default<internal::ParameterType::request, default_request_param>(
            std::tuple{},
            args...
        );

    [[maybe_unused]] int err = MPI_Ibarrier(
        mpi_communicator(),                       // comm
        &request_param.underlying().mpi_request() // request
    );
    this->mpi_error_hook(err, "MPI_Ibarrier");

    return internal::make_nonblocking_result<std::tuple<Args...>>(std::move(request_param));
}
/// @}
