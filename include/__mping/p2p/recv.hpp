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
#include <utility>

#include <kassert/kassert.hpp>
#include <mpi.h>

#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/error_handling.hpp"
#include "xxxxx-1ing/implementation_helpers.hpp"
#include "xxxxx-1ing/mpi_datatype.hpp"
#include "xxxxx-1ing/named_parameter_check.hpp"
#include "xxxxx-1ing/named_parameter_selection.hpp"
#include "xxxxx-1ing/named_parameter_types.hpp"
#include "xxxxx-1ing/named_parameters.hpp"
#include "xxxxx-1ing/p2p/helpers.hpp"
#include "xxxxx-1ing/p2p/probe.hpp"
#include "xxxxx-1ing/parameter_objects.hpp"
#include "xxxxx-1ing/result.hpp"
#include "xxxxx-1ing/status.hpp"

///// @addtogroup xxxxx-1ing_p2p
/// @{

// @brief Wrapper for \c MPI_Recv.
///
/// This wraps \c MPI_Recv. This operation performs a standard blocking receive.
/// If the \ref xxxxx-1ing::recv_counts() parameter is not specified, this first performs a probe, followed by a receive of
/// the probed message with the probed message size.
///
/// The following parameter is optional, but leads to an additional call to \c MPI_Probe if not present:
/// - \ref xxxxx-1ing::recv_count() the number of elements to receive. Will be probed before receiving if not given.
///
/// The following parameters are optional:
/// - \ref xxxxx-1ing::recv_buf() the buffer to receive the message into.  The buffer's underlying
/// storage must be large enough to hold all received elements. If no \ref xxxxx-1ing::recv_buf() is provided, the \c
/// value_type of the recv buffer has to be passed as a template parameter to \c recv().
///
//  - \ref xxxxx-1ing::recv_type() specifying the \c MPI datatype to use as recv type. If omitted, the \c MPI datatype is
/// derived automatically based on recv_buf's underlying \c value_type.
///
/// - \ref xxxxx-1ing::source() receive a message sent from this source rank. Defaults to probing for an arbitrary source,
/// i.e. \c source(rank::any).
///
/// - \ref xxxxx-1ing::tag() recv message with this tag. Defaults to receiving for an arbitrary tag, i.e. \c
/// tag(tags::any).
///
/// - \c xxxxx-1ing::status(ignore<>) or \ref xxxxx-1ing::status_out(). Returns info about the received message by setting the
/// appropriate fields in the status object passed by the user. If \ref xxxxx-1ing::status_out() is passed, constructs a
/// status object which may be retrieved by the user. The status can be ignored by passing \c
/// xxxxx-1ing::status(xxxxx-1ing::ignore<>). This is the default.
///
/// @tparam Args Automatically deduced template parameters.
/// @param args All required and any number of the optional parameters described above.
/// @return Result object wrapping the output parameters to be returned by value.
///
/// @see \ref docs/parameter_handling.md for general information about parameter handling in xxxxx-1.
/// <hr>
/// \include{doc} docs/resize_policy.dox
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename recv_value_type_tparam /* = xxxxx-1ing::internal::unused_tparam */, typename... Args>
auto xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::recv(Args... args) const {
    using namespace xxxxx-1ing::internal;
    xxxxx-1_CHECK_PARAMETERS(
        Args,
        xxxxx-1_REQUIRED_PARAMETERS(),
        xxxxx-1_OPTIONAL_PARAMETERS(recv_buf, tag, source, recv_count, recv_type, status)
    );
    using default_recv_buf_type = decltype(xxxxx-1ing::recv_buf(alloc_new<DefaultContainerType<recv_value_type_tparam>>));
    auto&& recv_buf =
        internal::select_parameter_type_or_default<internal::ParameterType::recv_buf, default_recv_buf_type>(
            std::tuple(),
            args...
        )
            .template construct_buffer_or_rebind<DefaultContainerType, internal::serialization_support_tag>();
    constexpr bool is_serialization_used = internal::buffer_uses_serialization<decltype(recv_buf)>;
    if constexpr (is_serialization_used) {
        xxxxx-1_UNSUPPORTED_PARAMETER(Args, recv_count, when using serialization);
        xxxxx-1_UNSUPPORTED_PARAMETER(Args, recv_type, when using serialization);
    }
    using recv_value_type = typename std::remove_reference_t<decltype(recv_buf)>::value_type;
    static_assert(
        !std::is_same_v<recv_value_type, internal::unused_tparam>,
        "No recv_buf parameter provided and no receive value given as template parameter. One of these is required."
    );

    auto&& recv_type = internal::determine_mpi_recv_datatype<recv_value_type, decltype(recv_buf)>(args...);
    [[maybe_unused]] constexpr bool recv_type_is_in_param = !internal::has_to_be_computed<decltype(recv_type)>;

    using default_source_buf_type = decltype(xxxxx-1ing::source(rank::any));

    auto&& source_param =
        internal::select_parameter_type_or_default<internal::ParameterType::source, default_source_buf_type>(
            {},
            args...
        );

    using default_tag_buf_type = decltype(xxxxx-1ing::tag(tags::any));

    auto&& tag_param =
        internal::select_parameter_type_or_default<internal::ParameterType::tag, default_tag_buf_type>({}, args...);

    constexpr auto tag_type = std::remove_reference_t<decltype(tag_param)>::tag_type;
    if constexpr (tag_type == internal::TagType::value) {
        int tag = tag_param.tag();
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

    // Get the optional recv_count parameter. If the parameter is not given,
    // allocate a new container.
    using default_recv_count_type = decltype(xxxxx-1ing::recv_count_out());
    auto&& recv_count_param =
        internal::select_parameter_type_or_default<internal::ParameterType::recv_count, default_recv_count_type>(
            std::tuple(),
            args...
        )
            .construct_buffer_or_rebind();

    KASSERT(internal::is_valid_rank_in_comm(source_param, *this, true, true));
    int source = source_param.rank_signed();
    int tag    = tag_param.tag();
    if constexpr (internal::has_to_be_computed<decltype(recv_count_param)>) {
        Status probe_status = this->probe(source_param.clone(), tag_param.clone(), status_out()).extract_status();
        source              = probe_status.source_signed();
        tag                 = probe_status.tag();
        recv_count_param.underlying() = asserting_cast<int>(probe_status.count(recv_type.get_single_element()));
    }

    // Ensure that we do not touch the recv buffer if MPI_PROC_NULL is passed,
    // because this is what the standard guarantees.
    if constexpr (std::remove_reference_t<decltype(source_param)>::rank_type != internal::RankType::null) {
        auto compute_required_recv_buf_size = [&] {
            return asserting_cast<size_t>(recv_count_param.get_single_element());
        };
        recv_buf.resize_if_requested(compute_required_recv_buf_size);
        KASSERT(
            // if the recv type is user provided, xxxxx-1ing cannot make any assumptions about the required size of the
            // recv buffer
            recv_type_is_in_param || recv_buf.size() >= compute_required_recv_buf_size(),
            "Recv buffer is not large enough to hold all received elements.",
            assert::light
        );
    }

    [[maybe_unused]] int err = MPI_Recv(
        recv_buf.data(),                             // buf
        recv_count_param.get_single_element(),       // count
        recv_type.get_single_element(),              // datatype
        source,                                      // source
        tag,                                         // tag
        this->mpi_communicator(),                    // comm
        internal::status_param_to_native_ptr(status) // status
    );
    this->mpi_error_hook(err, "MPI_Recv");

    return internal::make_mpi_result<std::tuple<Args...>>(
        deserialization_repack<is_serialization_used>(std::move(recv_buf)),
        std::move(recv_count_param),
        std::move(status),
        std::move(recv_type)
    );
}

/// @brief Convience wrapper for receiving single values via \c MPI_Recv.
///
/// This wraps \c MPI_Recv. This operation performs a standard blocking receive with a receive count of 1 and returns
/// the received value.
///
/// The following parameters are optional:
/// - \ref xxxxx-1ing::source() receive a message sent from this source rank. Defaults to
/// <code>xxxxx-1ing::source(xxxxx-1ing::rank::any)</code>.
/// - \ref xxxxx-1ing::tag() recv message with this tag. Defaults to receiving for an arbitrary tag, i.e.
/// <code>tag(tags::any)</code>.
/// - \ref xxxxx-1ing::status()  Returns info about the received message by setting the appropriate fields in the status
/// object passed by the user. The status can be ignored by passing \c xxxxx-1ing::status(xxxxx-1ing::ignore<>). This is the
/// default.
///
/// @tparam recv_value_type_tparam The type of the message to be received.
/// @tparam Args Automatically deduced template parameters.
/// @param args All required and any number of the optional buffers described above.
/// @return The received value of type \c recv_value_type_tparam.
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename recv_value_type_tparam, typename... Args>
auto xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::recv_single(Args... args) const {
    xxxxx-1_CHECK_PARAMETERS(Args, xxxxx-1_REQUIRED_PARAMETERS(), xxxxx-1_OPTIONAL_PARAMETERS(tag, source, status));

    using default_source_buf_type = decltype(xxxxx-1ing::source(rank::any));
    using source_param_type       = std::remove_reference_t<decltype(internal::select_parameter_type_or_default<
                                                               internal::ParameterType::source,
                                                               default_source_buf_type>({}, args...))>;
    static_assert(
        source_param_type::rank_type != internal::RankType::null,
        "You cannot receive an element from source xxxxx-1ing::rank::null."
    );

    using default_status_param_type = decltype(xxxxx-1ing::status(xxxxx-1ing::ignore<>));
    using status_param_type         = std::remove_reference_t<decltype(internal::select_parameter_type_or_default<
                                                               internal::ParameterType::status,
                                                               default_status_param_type>({}, args...))>;
    static_assert(
        !internal::is_extractable<status_param_type>,
        "xxxxx-1 cannot allocate a status object for you here, because we have no way of returning it. Pass a "
        "reference to a status object instead."
    );
    return recv(recv_count(1), recv_buf(alloc_new<recv_value_type_tparam>), std::forward<Args>(args)...);
}
/// @}
