// This file is part of xxxxx-1.
//
// Copyright 2022-2024 The xxxxx-1 Authors
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

#include <kassert/kassert.hpp>
#include <mpi.h>

#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/implementation_helpers.hpp"
#include "xxxxx-1ing/mpi_datatype.hpp"
#include "xxxxx-1ing/named_parameter_check.hpp"
#include "xxxxx-1ing/named_parameter_selection.hpp"
#include "xxxxx-1ing/named_parameter_types.hpp"
#include "xxxxx-1ing/named_parameters.hpp"
#include "xxxxx-1ing/p2p/helpers.hpp"
#include "xxxxx-1ing/parameter_objects.hpp"

///// @addtogroup xxxxx-1ing_p2p
/// @{

// @brief Wrapper for \c MPI_Send.
///
/// This wraps \c MPI_Send. This operation sends the elements in the input buffer provided via \c
/// xxxxx-1ing::send_buf() to the specified receiver rank using standard send mode.
/// The following parameters are required:
/// - \ref xxxxx-1ing::send_buf() containing the data that is sent.
///
/// - \ref xxxxx-1ing::send_count() specifying how many elements of the buffer are sent. If omitted, the size of the send
/// buffer is used as a default. This parameter is mandatory if \ref xxxxx-1ing::send_type() is given.
///
//  - \ref xxxxx-1ing::send_type() specifying the \c MPI datatype to use as send type. If omitted, the \c MPI datatype is
/// derived automatically based on send_buf's underlying \c value_type.
///
/// - \ref xxxxx-1ing::destination() the receiving rank.
///
/// The following parameters are optional:
/// - \ref xxxxx-1ing::tag() the tag added to the message. Defaults to the communicator's default tag (\ref
/// Communicator::default_tag()) if not present.
///
/// @tparam Args Automatically deduced template parameters.
/// @param args All required and any number of the optional parameters described above.
///
/// @see \ref docs/parameter_handling.md for general information about parameter handling in xxxxx-1.
/// <hr>
/// \include{doc} docs/resize_policy.dox
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename... Args>
void xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::send(Args... args) const {
    using namespace xxxxx-1ing::internal;
    xxxxx-1_CHECK_PARAMETERS(
        Args,
        xxxxx-1_REQUIRED_PARAMETERS(send_buf, destination),
        xxxxx-1_OPTIONAL_PARAMETERS(send_count, tag, send_mode, send_type)
    );

    auto&& send_buf = internal::select_parameter_type<internal::ParameterType::send_buf>(args...)
                          .template construct_buffer_or_rebind<UnusedRebindContainer, serialization_support_tag>();
    constexpr bool is_serialization_used = internal::buffer_uses_serialization<decltype(send_buf)>;
    if constexpr (is_serialization_used) {
        xxxxx-1_UNSUPPORTED_PARAMETER(Args, send_count, when using serialization);
        xxxxx-1_UNSUPPORTED_PARAMETER(Args, send_type, when using serialization);
        send_buf.underlying().serialize();
    }
    using send_value_type = typename std::remove_reference_t<decltype(send_buf)>::value_type;

    auto&& send_type = internal::determine_mpi_send_datatype<send_value_type>(args...);

    using default_send_count_type = decltype(xxxxx-1ing::send_count_out());
    auto&& send_count =
        internal::select_parameter_type_or_default<internal::ParameterType::send_count, default_send_count_type>(
            {},
            args...
        )
            .construct_buffer_or_rebind();
    if constexpr (has_to_be_computed<decltype(send_count)>) {
        send_count.underlying() = asserting_cast<int>(send_buf.size());
    }

    auto const&    destination = internal::select_parameter_type<internal::ParameterType::destination>(args...);
    constexpr auto rank_type   = std::remove_reference_t<decltype(destination)>::rank_type;
    static_assert(
        rank_type == RankType::value || rank_type == RankType::null,
        "Please provide an explicit destination or destination(ranks::null)."
    );

    using default_tag_buf_type = decltype(xxxxx-1ing::tag(this->default_tag()));

    auto&& tag_param = internal::select_parameter_type_or_default<internal::ParameterType::tag, default_tag_buf_type>(
        std::tuple(this->default_tag()),
        args...
    );

    // this ensures that the user does not try to pass MPI_ANY_TAG, which is not allowed for sends
    static_assert(
        std::remove_reference_t<decltype(tag_param)>::tag_type == TagType::value,
        "Please provide a tag for the message."
    );
    int tag = tag_param.tag();
    KASSERT(
        Environment<>::is_valid_tag(tag),
        "invalid tag " << tag << ", must be in range [0, " << Environment<>::tag_upper_bound() << "]"
    );

    using send_mode_obj_type = decltype(internal::select_parameter_type_or_default<
                                        internal::ParameterType::send_mode,
                                        internal::SendModeParameter<internal::standard_mode_t>>(std::tuple(), args...));
    using send_mode          = typename std::remove_reference_t<send_mode_obj_type>::send_mode;

    // RankType::null is valid, RankType::any is not.
    KASSERT(is_valid_rank_in_comm(destination, *this, true, false), "Invalid destination rank.");

    if constexpr (std::is_same_v<send_mode, internal::standard_mode_t>) {
        [[maybe_unused]] int err = MPI_Send(
            send_buf.data(),                 // send_buf
            send_count.get_single_element(), // send_count
            send_type.get_single_element(),  // send_type
            destination.rank_signed(),       // destination
            tag,                             // tag
            this->mpi_communicator()
        );
        this->mpi_error_hook(err, "MPI_Send");
    } else if constexpr (std::is_same_v<send_mode, internal::buffered_mode_t>) {
        [[maybe_unused]] int err = MPI_Bsend(
            send_buf.data(),                 // send_buf
            send_count.get_single_element(), // send_count
            send_type.get_single_element(),  // send_type
            destination.rank_signed(),       // destination
            tag,                             // tag
            this->mpi_communicator()
        );
        this->mpi_error_hook(err, "MPI_Bsend");
    } else if constexpr (std::is_same_v<send_mode, internal::synchronous_mode_t>) {
        [[maybe_unused]] int err = MPI_Ssend(
            send_buf.data(),                 // send_buf
            send_count.get_single_element(), // send_count
            send_type.get_single_element(),  // send_type
            destination.rank_signed(),       // destination
            tag,                             // tag
            this->mpi_communicator()
        );
        this->mpi_error_hook(err, "MPI_Ssend");
    } else if constexpr (std::is_same_v<send_mode, internal::ready_mode_t>) {
        [[maybe_unused]] int err = MPI_Rsend(
            send_buf.data(),                 // send_buf
            send_count.get_single_element(), // send_count
            send_type.get_single_element(),  // send_type
            destination.rank_signed(),       // destination
            tag,                             // tag
            this->mpi_communicator()
        );
        this->mpi_error_hook(err, "MPI_Rsend");
    }
}

/// @brief Convenience wrapper for MPI_Bsend. Calls \ref xxxxx-1ing::Communicator::send() with the appropriate send mode
/// set.
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename... Args>
void xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::bsend(Args... args) const {
    this->send(std::forward<Args>(args)..., send_mode(send_modes::buffered));
}

/// @brief Convenience wrapper for MPI_Ssend. Calls \ref xxxxx-1ing::Communicator::send() with the appropriate send mode
/// set.
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename... Args>
void xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::ssend(Args... args) const {
    this->send(std::forward<Args>(args)..., send_mode(send_modes::synchronous));
}

/// @brief Convenience wrapper for MPI_Rsend. Calls \ref xxxxx-1ing::Communicator::send() with the appropriate send mode
/// set.
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename... Args>
void xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::rsend(Args... args) const {
    this->send(std::forward<Args>(args)..., send_mode(send_modes::ready));
}
/// @}
