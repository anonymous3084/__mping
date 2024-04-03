// This file is part of xxxxx-1.
//
// Copyright 2023 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version. xxxxx-1 is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with xxxxx-1.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <type_traits>

#include <mpi.h>

#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/implementation_helpers.hpp"
#include "xxxxx-1ing/named_parameter_check.hpp"
#include "xxxxx-1ing/named_parameter_selection.hpp"
#include "xxxxx-1ing/named_parameter_types.hpp"
#include "xxxxx-1ing/named_parameters.hpp"
#include "xxxxx-1ing/result.hpp"

//// @addtogroup xxxxx-1ing_p2p
/// @{

// @brief Wrapper for \c MPI_Iprobe.
///
/// This wraps \c MPI_Iprobe. This operation checks if there is a message matching the (optionally) specified source
/// and tag that can be received, and returns a \c bool indicating whether a message matched by default.
/// If the user passes \ref xxxxx-1ing::status_out(), returns a \c std::optional containing an \ref xxxxx-1ing::MPIResult,
/// which encapsulates a status object. If the probe does not match any message, returns \c std::nullopt.
///
/// The following parameters are optional:
/// - \ref xxxxx-1ing::tag() probe for messages with this tag. Defaults to probing for an arbitrary tag, i.e. \c
/// tag(tags::any).
/// - \ref xxxxx-1ing::source() probe for messages sent from this source rank.
/// Defaults to probing for an arbitrary source, i.e. \c source(rank::any).
/// - \ref xxxxx-1ing::status() or \ref xxxxx-1ing::status_out(). Returns info about
/// the probed message by setting the appropriate fields in the status object
/// passed by the user. If \ref xxxxx-1ing::status_out() is passed, constructs a
/// status object which may be retrieved by the user.
/// The status can be ignored by passing \c xxxxx-1ing::status(xxxxx-1ing::ignore<>). This is the default.
///
/// @tparam Args Automatically deduced template parameters.
/// @param args All required and any number of the optional buffers described
/// above.
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename... Args>
auto xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::iprobe(Args... args) const {
    xxxxx-1_CHECK_PARAMETERS(Args, xxxxx-1_REQUIRED_PARAMETERS(), xxxxx-1_OPTIONAL_PARAMETERS(tag, source, status));

    using default_source_buf_type = decltype(xxxxx-1ing::source(rank::any));

    auto&& source =
        internal::select_parameter_type_or_default<internal::ParameterType::source, default_source_buf_type>(
            {},
            args...
        );

    using default_tag_buf_type = decltype(xxxxx-1ing::tag(tags::any));

    auto&& tag_param =
        internal::select_parameter_type_or_default<internal::ParameterType::tag, default_tag_buf_type>({}, args...);
    int tag = tag_param.tag();

    constexpr auto tag_type = std::remove_reference_t<decltype(tag_param)>::tag_type;
    if constexpr (tag_type == internal::TagType::value) {
        KASSERT(
            Environment<>::is_valid_tag(tag),
            "invalid tag " << tag << ", must be in range [0, " << Environment<>::tag_upper_bound() << "]"
        );
    }

    using default_status_param_type = decltype(xxxxx-1ing::status(xxxxx-1ing::ignore<>));

    auto&& status =
        internal::select_parameter_type_or_default<internal::ParameterType::status, default_status_param_type>(
            {},
            args...
        )
            .construct_buffer_or_rebind();

    KASSERT(internal::is_valid_rank_in_comm(source, *this, true, true), "Invalid source rank.");

    int                  flag;
    [[maybe_unused]] int err = MPI_Iprobe(
        source.rank_signed(),                        // source
        tag,                                         // tag
        this->mpi_communicator(),                    // comm
        &flag,                                       // flag
        internal::status_param_to_native_ptr(status) // status
    );
    this->mpi_error_hook(err, "MPI_Iprobe");

    // if xxxxx-1 owns the status (i.e. when the user passed status_out()) we
    // return an optional, containing the status, otherwise just a bool
    // indicating probe success.
    if constexpr (internal::is_extractable<std::remove_reference_t<decltype(status)>>) {
        if (flag) {
            return std::optional{internal::make_mpi_result<std::tuple<Args...>>(std::move(status))};
        } else {
            return std::optional<decltype(internal::make_mpi_result<std::tuple<Args...>>(std::move(status)))>{};
        }
    } else {
        return static_cast<bool>(flag);
    }
}
/// @}
