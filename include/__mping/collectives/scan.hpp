
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

#include <kassert/kassert.hpp>
#include <mpi.h>

#include "xxxxx-1ing/assertion_levels.hpp"
#include "xxxxx-1ing/checking_casts.hpp"
#include "xxxxx-1ing/collectives/collectives_helpers.hpp"
#include "xxxxx-1ing/comm_helper/is_same_on_all_ranks.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/error_handling.hpp"
#include "xxxxx-1ing/mpi_datatype.hpp"
#include "xxxxx-1ing/named_parameter_check.hpp"
#include "xxxxx-1ing/named_parameter_selection.hpp"
#include "xxxxx-1ing/named_parameter_types.hpp"
#include "xxxxx-1ing/named_parameters.hpp"
#include "xxxxx-1ing/result.hpp"

//// @addtogroup xxxxx-1ing_collectives
/// @{

// @brief Wrapper for \c MPI_Scan.
///
/// This wraps \c MPI_Scan, which is used to perform an inclusive prefix reduction on data distributed across the
/// calling processes. / \c scan() returns in \c recv_buf of the process with rank \c i, the reduction (calculated
/// according to the function op) of the values in the sendbufs of processes with ranks \f$0, ..., i\f$ (inclusive).
///
/// The following parameters are required:
/// - \ref xxxxx-1ing::send_buf() containing the data for which to perform the scan. This buffer has to be the
///  same size at each rank.
///
/// - \ref xxxxx-1ing::op() wrapping the operation to apply to the input. If \ref xxxxx-1ing::send_recv_type() is provided
/// explicitly, the compatibility of the type and operation has to be ensured by the user.
///
/// The following parameters are optional:
/// - \ref xxxxx-1ing::recv_buf() containing a buffer for the output. A buffer size of at least `send_recv_count` elements
/// is required.
///
/// - \ref xxxxx-1ing::send_recv_count() containing the number of elements to be processed in this operation. This
/// parameter has to be the same at each rank. If omitted, the size of the send buffer will be used as send_recv_count.
///
/// - \ref xxxxx-1ing::send_recv_type() specifying the \c MPI datatype to use as data type in this operation. If omitted,
/// the \c MPI datatype is derived automatically based on send_buf's underlying \c value_type.
///
/// In-place scan is supported by providing `send_recv_buf()` instead of `send_buf()` and `recv_buf()`. For details
/// on the in-place version, see \ref Communicator::scan_inplace().
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
template <typename... Args>
auto xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::scan(Args... args) const {
    using namespace xxxxx-1ing::internal;
    constexpr bool inplace = internal::has_parameter_type<internal::ParameterType::send_recv_buf, Args...>();
    if constexpr (inplace) {
        return this->scan_inplace(std::forward<Args>(args)...);
    } else {
        xxxxx-1_CHECK_PARAMETERS(
            Args,
            xxxxx-1_REQUIRED_PARAMETERS(send_buf, op),
            xxxxx-1_OPTIONAL_PARAMETERS(send_recv_count, recv_buf, send_recv_type)
        );

        // get the send buffer and deduce the send and recv value types.
        auto const&& send_buf = select_parameter_type<ParameterType::send_buf>(args...).construct_buffer_or_rebind();
        using send_value_type = typename std::remove_reference_t<decltype(send_buf)>::value_type;
        using default_recv_value_type = std::remove_const_t<send_value_type>;

        // deduce the recv buffer type and get (if provided) the recv buffer or allocate one (if not provided).
        using default_recv_buf_type =
            decltype(xxxxx-1ing::recv_buf(alloc_new<DefaultContainerType<default_recv_value_type>>));
        auto&& recv_buf =
            select_parameter_type_or_default<ParameterType::recv_buf, default_recv_buf_type>(std::tuple(), args...)
                .template construct_buffer_or_rebind<DefaultContainerType>();

        // get the send_recv_type
        auto&& send_recv_type = determine_mpi_send_recv_datatype<send_value_type, decltype(recv_buf)>(args...);
        [[maybe_unused]] constexpr bool send_recv_type_is_in_param = !has_to_be_computed<decltype(send_recv_type)>;

        // get the send_recv count
        using default_send_recv_count_type = decltype(xxxxx-1ing::send_recv_count_out());
        auto&& send_recv_count             = internal::select_parameter_type_or_default<
                                     internal::ParameterType::send_recv_count,
                                     default_send_recv_count_type>(std::tuple(), args...)
                                     .construct_buffer_or_rebind();

        constexpr bool do_compute_send_recv_count = internal::has_to_be_computed<decltype(send_recv_count)>;
        if constexpr (do_compute_send_recv_count) {
            send_recv_count.underlying() = asserting_cast<int>(send_buf.size());
        }

        KASSERT(
            is_same_on_all_ranks(send_recv_count.get_single_element()),
            "The send_recv_count has to be the same on all ranks.",
            assert::light_communication
        );

        // get the operation used for the reduction. The signature of the provided function is checked while building.
        auto& operation_param = select_parameter_type<ParameterType::op>(args...);
        auto  operation       = operation_param.template build_operation<send_value_type>();

        auto compute_required_recv_buf_size = [&]() {
            return asserting_cast<size_t>(send_recv_count.get_single_element());
        };
        recv_buf.resize_if_requested(compute_required_recv_buf_size);
        KASSERT(
            // if the send_recv type is user provided, xxxxx-1ing cannot make any assumptions about the required size of
            // the recv buffer
            send_recv_type_is_in_param || recv_buf.size() >= compute_required_recv_buf_size(),
            "Recv buffer is not large enough to hold all received elements.",
            assert::light
        );

        // Perform the MPI_Scan call and return.
        [[maybe_unused]] int err = MPI_Scan(
            send_buf.data(),                      // sendbuf
            recv_buf.data(),                      // recvbuf,
            send_recv_count.get_single_element(), // count
            send_recv_type.get_single_element(),  // datatype,
            operation.op(),                       // op
            mpi_communicator()                    // communicator
        );

        this->mpi_error_hook(err, "MPI_Scan");
        return make_mpi_result<std::tuple<Args...>>(
            std::move(recv_buf),
            std::move(send_recv_type),
            std::move(send_recv_count)
        );
    }
}

/// @brief Wrapper for the in-place version of \ref Communicator::scan().
///
/// This variant must be called collectively by all ranks in the communicator. It is semantically equivalent to \ref
/// Communicator::scan(), but the input buffer is used as the output buffer. This means that the input buffer is
/// overwritten with the result of the scan.
///
/// The following parameters are required:
/// - \ref xxxxx-1ing::send_recv_buf() containing the data for which to perform the scan and will store the result
/// of the scan.
///
/// - \ref xxxxx-1ing::op() wrapping the operation to apply to the input. If \ref xxxxx-1ing::send_recv_type() is provided
/// explicitly, the compatibility of the type and operation has to be ensured by the user.
///
/// The following parameters are optional:
/// - \ref xxxxx-1ing::send_recv_count() containing the number of elements to be processed in this operation. This
/// parameter has to be the same at each rank. If omitted, the size of the send buffer will be used as send_recv_count.
///
/// - \ref xxxxx-1ing::send_recv_type() specifying the \c MPI datatype to use as data type in this operation. If omitted,
/// the \c MPI datatype is derived automatically based on send_recv_buf's underlying \c value_type.
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
template <typename... Args>
auto xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::scan_inplace(Args... args) const {
    using namespace xxxxx-1ing::internal;
    xxxxx-1_CHECK_PARAMETERS(
        Args,
        xxxxx-1_REQUIRED_PARAMETERS(send_recv_buf, op),
        xxxxx-1_OPTIONAL_PARAMETERS(send_recv_count, send_recv_type)
    );

    // get the send recv buffer and deduce the send and recv value types.
    auto&& send_recv_buf = select_parameter_type<ParameterType::send_recv_buf>(args...).construct_buffer_or_rebind();
    using value_type     = typename std::remove_reference_t<decltype(send_recv_buf)>::value_type;

    // get the send_recv_type
    auto&& type = determine_mpi_send_recv_datatype<value_type, decltype(send_recv_buf)>(args...);
    [[maybe_unused]] constexpr bool type_is_in_param = !has_to_be_computed<decltype(type)>;

    // get the send_recv count
    using default_count_type = decltype(xxxxx-1ing::send_recv_count_out());
    auto&& count =
        internal::select_parameter_type_or_default<internal::ParameterType::send_recv_count, default_count_type>(
            std::tuple(),
            args...
        )
            .construct_buffer_or_rebind();

    constexpr bool do_compute_count = internal::has_to_be_computed<decltype(count)>;
    if constexpr (do_compute_count) {
        count.underlying() = asserting_cast<int>(send_recv_buf.size());
    }

    KASSERT(
        is_same_on_all_ranks(count.get_single_element()),
        "The send_recv_count has to be the same on all ranks.",
        assert::light_communication
    );

    // get the operation used for the reduction. The signature of the provided function is checked while building.
    auto& operation_param = select_parameter_type<ParameterType::op>(args...);
    auto  operation       = operation_param.template build_operation<value_type>();

    auto compute_required_recv_buf_size = [&]() {
        return asserting_cast<size_t>(count.get_single_element());
    };
    send_recv_buf.resize_if_requested(compute_required_recv_buf_size);
    KASSERT(
        // if the send_recv type is user provided, xxxxx-1ing cannot make any assumptions about the required size of
        // the buffer
        type_is_in_param || send_recv_buf.size() >= compute_required_recv_buf_size(),
        "Send/Recv buffer is not large enough to hold all received elements.",
        assert::light
    );

    // Perform the MPI_Scan call and return.
    [[maybe_unused]] int err = MPI_Scan(
        MPI_IN_PLACE,               // sendbuf
        send_recv_buf.data(),       // recvbuf,
        count.get_single_element(), // count
        type.get_single_element(),  // datatype,
        operation.op(),             // op
        mpi_communicator()          // communicator
    );

    this->mpi_error_hook(err, "MPI_Scan");
    return make_mpi_result<std::tuple<Args...>>(std::move(send_recv_buf), std::move(type), std::move(count));
}

/// @brief Wrapper for \c MPI_Scan for single elements.
///
/// This is functionally equivalent to \c scan() but provided for uniformity with other operations (e.g. \c
/// bcast_single()). \c scan_single() wraps \c MPI_Scan, which is used to perform an inclusive prefix reduction on data
/// distributed across the calling processes. \c scan() returns on the process with rank \f$i\f$, the
/// reduction (calculated according to the function op) of the values in the sendbufs of processes with ranks \f$0, ...,
/// i\f$ (inclusive).
///
/// The following parameters are required:
/// - xxxxx-1ing::send_buf() containing the data for which to perform the scan. This buffer has to be a single element on
/// each rank.
/// - xxxxx-1ing::op() wrapping the operation to apply to the input.
///
/// @tparam Args Automatically deduced template parameters.
/// @param args All required and any number of the optional buffers described above.
/// @return The single element result of the scan.
template <
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename... Plugins>
template <typename... Args>
auto xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::scan_single(Args... args) const {
    //! If you expand this function to not being only a simple wrapper around scan, you have to write more unit
    //! tests!

    using namespace xxxxx-1ing::internal;

    // The send and recv buffers are always of the same size in scan, thus, there is no additional exchange of
    // recv_counts.
    xxxxx-1_CHECK_PARAMETERS(Args, xxxxx-1_REQUIRED_PARAMETERS(send_buf, op), xxxxx-1_OPTIONAL_PARAMETERS());

    using send_buf_type = buffer_type_with_requested_parameter_type<ParameterType::send_buf, Args...>;
    static_assert(
        send_buf_type::is_single_element,
        "The underlying container has to be a single element \"container\""
    );
    using value_type = typename send_buf_type::value_type;
    return this->scan(recv_buf(alloc_new<value_type>), std::forward<Args>(args)...);
}
/// @}
