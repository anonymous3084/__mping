// This file is part of xxxxx-1.
//
// Copyright 2022-2023 The xxxxx-1 Authors
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

#include <cstddef>
#include <numeric>

#include <kassert/kassert.hpp>
#include <mpi.h>

#include "xxxxx-1ing/assertion_levels.hpp"
#include "xxxxx-1ing/checking_casts.hpp"
#include "xxxxx-1ing/collectives/collectives_helpers.hpp"
#include "xxxxx-1ing/comm_helper/is_same_on_all_ranks.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/mpi_datatype.hpp"
#include "xxxxx-1ing/named_parameter_check.hpp"
#include "xxxxx-1ing/named_parameter_selection.hpp"
#include "xxxxx-1ing/named_parameter_types.hpp"
#include "xxxxx-1ing/named_parameters.hpp"
#include "xxxxx-1ing/result.hpp"

/// @addtogroup xxxxx-1ing_collectives
/// @{

/// @brief Wrapper for \c MPI_Gather.
///
/// This wrapper for \c MPI_Gather collects the same amount of data from each rank to a root.
///
/// The following arguments are required:
/// - \ref xxxxx-1ing::send_buf() containing the data that is sent to the root.
///
/// The following buffers are optional:
/// - \ref xxxxx-1ing::send_count() [on all PEs] specifying the number of elements to send to the root PE. If not given,
/// the size of the xxxxx-1ing::send_buf() will be used. This parameter is mandatory if \ref xxxxx-1ing::send_type() is given.
///
/// - \ref xxxxx-1ing::send_type() specifying the \c MPI datatype to use as send type. If omitted, the \c MPI datatype is
/// derived automatically based on send_buf's underlying \c value_type. This parameter is ignored on non-root ranks.
///
/// - \ref xxxxx-1ing::recv_buf() containing a buffer for the output.
/// On the root rank, the buffer will contain all data from all send buffers.
/// At all other ranks, the buffer will not be modified and the parameter is ignored.
///
/// - \ref xxxxx-1ing::recv_count() [on root PE] specifying the number of elements to receive from each PE. On non-root
/// ranks, this parameter is ignored. If not specified, defaults to the value of \ref xxxxx-1ing::send_count() on the
/// root. PE. In total, comm.size() * recv_counts elements will be received into the receiver buffer.
/// This parameter is mandatory if \ref xxxxx-1ing::recv_type() is given.
///
/// - \ref xxxxx-1ing::recv_type() specifying the \c MPI datatype to use as recv type. If omitted, the \c MPI datatype is
/// derived automatically based on recv_buf's underlying \c value_type.
///
/// - \ref xxxxx-1ing::root() specifying an alternative root. If not present, the default root of the \c Communicator
/// is used, see root().
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
auto xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::gather(Args... args) const {
    using namespace xxxxx-1ing::internal;
    xxxxx-1_CHECK_PARAMETERS(
        Args,
        xxxxx-1_REQUIRED_PARAMETERS(send_buf),
        xxxxx-1_OPTIONAL_PARAMETERS(send_count, recv_buf, recv_count, root, send_type, recv_type)
    );

    auto&& root = internal::select_parameter_type_or_default<internal::ParameterType::root, internal::RootDataBuffer>(
        std::tuple(this->root()),
        args...
    );
    KASSERT(this->is_valid_rank(root.rank_signed()), "Invalid rank as root.");
    KASSERT(
        this->is_same_on_all_ranks(root.rank_signed()),
        "Root has to be the same on all ranks.",
        assert::light_communication
    );

    auto&& send_buf =
        internal::select_parameter_type<internal::ParameterType::send_buf>(args...).construct_buffer_or_rebind();
    using send_value_type = typename std::remove_reference_t<decltype(send_buf)>::value_type;

    using default_send_count_type = decltype(xxxxx-1ing::send_count_out());
    auto&& send_count =
        internal::select_parameter_type_or_default<internal::ParameterType::send_count, default_send_count_type>(
            std::tuple(),
            args...
        )
            .construct_buffer_or_rebind();
    constexpr bool do_compute_send_count = internal::has_to_be_computed<decltype(send_count)>;
    if constexpr (do_compute_send_count) {
        send_count.underlying() = asserting_cast<int>(send_buf.size());
    }

    using default_recv_buf_type = decltype(xxxxx-1ing::recv_buf(alloc_new<DefaultContainerType<send_value_type>>));

    auto&& recv_buf =
        internal::select_parameter_type_or_default<internal::ParameterType::recv_buf, default_recv_buf_type>(
            std::tuple(),
            args...
        )
            .template construct_buffer_or_rebind<DefaultContainerType>();
    using recv_value_type = typename std::remove_reference_t<decltype(recv_buf)>::value_type;

    // Get send_type and recv_type
    auto&& [send_type, recv_type] =
        internal::determine_mpi_datatypes<send_value_type, recv_value_type, decltype(recv_buf)>(args...);
    [[maybe_unused]] constexpr bool recv_type_is_in_param = !has_to_be_computed<decltype(recv_type)>;

    // Optional parameter: recv_count()
    // Default: compute value based on send_buf.size on root
    using default_recv_count_type = decltype(xxxxx-1ing::recv_count_out());
    auto&& recv_count =
        internal::select_parameter_type_or_default<internal::ParameterType::recv_count, default_recv_count_type>(
            std::tuple(),
            args...
        )
            .construct_buffer_or_rebind();
    constexpr bool do_compute_recv_count = has_to_be_computed<decltype(recv_count)>;
    if constexpr (do_compute_recv_count) {
        if (this->is_root(root.rank_signed())) {
            recv_count.underlying() = send_count.get_single_element();
        }
    }

    auto compute_required_recv_buf_size = [&] {
        return asserting_cast<size_t>(recv_count.get_single_element()) * this->size();
    };
    if (this->is_root(root.rank_signed())) {
        recv_buf.resize_if_requested(compute_required_recv_buf_size);
        KASSERT(
            // if the recv type is user provided, xxxxx-1ing cannot make any assumptions about the required size of
            // the recv buffer
            recv_type_is_in_param || recv_buf.size() >= compute_required_recv_buf_size(),
            "Recv buffer is not large enough to hold all received elements.",
            assert::light
        );
    }

    // error code can be unused if KTHROW is removed at compile time
    [[maybe_unused]] int err = MPI_Gather(
        send_buf.data(),                 // sendbuffer
        send_count.get_single_element(), // sendcount
        send_type.get_single_element(),  // sendtype
        recv_buf.data(),                 // recvbuffer
        recv_count.get_single_element(), // recvcount
        recv_type.get_single_element(),  // recvtype
        root.rank_signed(),              // root
        this->mpi_communicator()         // communicator
    );
    this->mpi_error_hook(err, "MPI_Gather");
    return make_mpi_result<std::tuple<Args...>>(
        std::move(recv_buf),
        std::move(recv_count),
        std::move(send_count),
        std::move(send_type),
        std::move(recv_type)
    );
}

/// @brief Wrapper for \c MPI_Gatherv.
///
/// This wrapper for \c MPI_Gatherv collects possibly different amounts of data from each rank to a root.
///
/// The following arguments are required:
/// - \ref xxxxx-1ing::send_buf() containing the data that is sent to the root.
///
/// The following parameter is optional but results in communication overhead if omitted:
/// - \ref xxxxx-1ing::recv_counts() containing the number of elements to receive from each rank. Only the root rank uses
/// the content of this buffer, all other ranks ignore it. However, if provided on any rank it must be provided on all
/// ranks (possibly empty on non-root ranks). If each rank provides this parameter either as an output parameter or by
/// passing \c recv_counts(xxxxx-1ing::ignore), then the \c recv_counts on root will be computed by a gather of all local
/// send counts. This parameter is mandatory (as an in-parameter) if \ref xxxxx-1ing::recv_type() is given.
///
/// The following buffers are optional:
/// - \ref xxxxx-1ing::send_count() [on all PEs] specifying the number of elements to send to the root rank. If not given,
/// the size of the xxxxx-1ing::send_buf() will be used. This parameter is mandatory if \ref xxxxx-1ing::send_type() is given.
///
/// - \ref xxxxx-1ing::recv_buf() containing a buffer for the output. Afterwards, at the root, this buffer will contain
/// all data from all send buffers. At all other ranks, the buffer will have size 0.
///
/// - \ref xxxxx-1ing::recv_displs() containing the offsets of the messages in recv_buf. The `recv_counts[i]` elements
/// starting at `recv_buf[recv_displs[i]]` will be received from rank `i`. If omitted, this is calculated as the
/// exclusive prefix-sum of `recv_counts`.
///
/// - \ref xxxxx-1ing::root() specifying an alternative root. If not present, the default root of the \c Communicator
/// is used, see root().
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
auto xxxxx-1ing::Communicator<DefaultContainerType, Plugins...>::gatherv(Args... args) const {
    using namespace xxxxx-1ing::internal;
    xxxxx-1_CHECK_PARAMETERS(
        Args,
        xxxxx-1_REQUIRED_PARAMETERS(send_buf),
        xxxxx-1_OPTIONAL_PARAMETERS(recv_buf, root, send_count, recv_counts, recv_displs, send_type, recv_type)
    );

    // get send buffer
    auto&& send_buf =
        internal::select_parameter_type<internal::ParameterType::send_buf>(args...).construct_buffer_or_rebind();
    using send_value_type = typename std::remove_reference_t<decltype(send_buf)>::value_type;

    // get recv buffer
    using default_recv_buf_type = decltype(xxxxx-1ing::recv_buf(alloc_new<DefaultContainerType<send_value_type>>));
    auto&& recv_buf =
        internal::select_parameter_type_or_default<internal::ParameterType::recv_buf, default_recv_buf_type>(
            std::tuple(),
            args...
        )
            .template construct_buffer_or_rebind<DefaultContainerType>();
    using recv_value_type = typename std::remove_reference_t<decltype(recv_buf)>::value_type;

    // get root rank
    auto&& root = internal::select_parameter_type_or_default<internal::ParameterType::root, internal::RootDataBuffer>(
        std::tuple(this->root()),
        args...
    );

    // get send and recv type
    auto&& [send_type, recv_type] =
        internal::determine_mpi_datatypes<send_value_type, recv_value_type, decltype(recv_buf)>(args...);
    [[maybe_unused]] constexpr bool recv_type_is_in_param = !has_to_be_computed<decltype(recv_type)>;

    // get recv counts
    using default_recv_counts_type = decltype(xxxxx-1ing::recv_counts_out(alloc_new<DefaultContainerType<int>>));
    auto&& recv_counts =
        internal::select_parameter_type_or_default<internal::ParameterType::recv_counts, default_recv_counts_type>(
            std::tuple(),
            args...
        )
            .template construct_buffer_or_rebind<DefaultContainerType>();
    using recv_counts_type = typename std::remove_reference_t<decltype(recv_counts)>::value_type;
    static_assert(std::is_same_v<std::remove_const_t<recv_counts_type>, int>, "Recv counts must be of type int");
    using recv_counts_param_type = std::remove_reference_t<decltype(recv_counts)>;
    constexpr bool recv_counts_is_ignore =
        is_empty_data_buffer_v<
            recv_counts_param_type> && recv_counts_param_type::buffer_type == internal::BufferType::ignore;

    // because this check is asymmetric, we move it before any communication happens.
    KASSERT(!this->is_root(root.rank_signed()) || !recv_counts_is_ignore, "Root cannot ignore recv counts.");

    KASSERT(this->is_valid_rank(root.rank_signed()), "Invalid rank as root.");
    KASSERT(
        this->is_same_on_all_ranks(root.rank_signed()),
        "Root has to be the same on all ranks.",
        assert::light_communication
    );

    using default_send_count_type = decltype(xxxxx-1ing::send_count_out());
    auto&& send_count =
        internal::select_parameter_type_or_default<internal::ParameterType::send_count, default_send_count_type>(
            std::tuple(),
            args...
        )
            .construct_buffer_or_rebind();
    constexpr bool do_compute_send_count = internal::has_to_be_computed<decltype(send_count)>;
    if constexpr (do_compute_send_count) {
        send_count.underlying() = asserting_cast<int>(send_buf.size());
    }

    // get recv displs
    using default_recv_displs_type = decltype(xxxxx-1ing::recv_displs_out(alloc_new<DefaultContainerType<int>>));
    auto&& recv_displs =
        internal::select_parameter_type_or_default<internal::ParameterType::recv_displs, default_recv_displs_type>(
            std::tuple(),
            args...
        )
            .template construct_buffer_or_rebind<DefaultContainerType>();
    using recv_displs_type = typename std::remove_reference_t<decltype(recv_displs)>::value_type;
    static_assert(std::is_same_v<std::remove_const_t<recv_displs_type>, int>, "Recv displs must be of type int");

    // calculate recv_counts if necessary
    constexpr bool do_calculate_recv_counts =
        internal::has_to_be_computed<decltype(recv_counts)> || recv_counts_is_ignore;
    KASSERT(
        is_same_on_all_ranks(do_calculate_recv_counts),
        "Receive counts are given on some ranks and are omitted on others",
        assert::light_communication
    );

    auto compute_required_recv_counts_size = [&] {
        return asserting_cast<size_t>(this->size());
    };
    if constexpr (do_calculate_recv_counts) {
        if (this->is_root(root.rank_signed())) {
            recv_counts.resize_if_requested(compute_required_recv_counts_size);
            KASSERT(
                recv_counts.size() >= compute_required_recv_counts_size(),
                "Recv counts buffer is smaller than the number of PEs at the root PE.",
                assert::light
            );
        }
        this->gather(
            xxxxx-1ing::send_buf(send_count.underlying()),
            xxxxx-1ing::recv_buf(recv_counts.get()),
            xxxxx-1ing::send_count(1),
            xxxxx-1ing::recv_count(1),
            xxxxx-1ing::root(root.rank_signed())
        );
    } else {
        if (this->is_root(root.rank_signed())) {
            KASSERT(
                recv_counts.size() >= compute_required_recv_counts_size(),
                "Recv counts buffer is smaller than the number of PEs at the root PE.",
                assert::light
            );
        }
    }

    using default_recv_buf_type = decltype(xxxxx-1ing::recv_buf(alloc_new<DefaultContainerType<send_value_type>>));

    // calculate recv_displs if necessary
    constexpr bool do_calculate_recv_displs          = internal::has_to_be_computed<decltype(recv_displs)>;
    auto           compute_required_recv_displs_size = [&] {
        return asserting_cast<size_t>(this->size());
    };
    if constexpr (do_calculate_recv_displs) {
        if (this->is_root(root.rank_signed())) {
            recv_displs.resize_if_requested(compute_required_recv_displs_size);
            std::exclusive_scan(recv_counts.data(), recv_counts.data() + this->size(), recv_displs.data(), 0);
        }
    }
    if (this->is_root(root.rank_signed())) {
        KASSERT(
            recv_displs.size() >= compute_required_recv_displs_size(),
            "Recv displs buffer is smaller than the number of PEs at the root PE.",
            assert::light
        );
    }

    if (this->is_root(root.rank_signed())) {
        auto compute_required_recv_buf_size = [&] {
            return compute_required_recv_buf_size_in_vectorized_communication(recv_counts, recv_displs, this->size());
        };
        recv_buf.resize_if_requested(compute_required_recv_buf_size);
        KASSERT(
            // if the recv type is user provided, xxxxx-1ing cannot make any assumptions about the required size of
            // the recv buffer
            recv_type_is_in_param || recv_buf.size() >= compute_required_recv_buf_size(),
            "Recv buffer is not large enough to hold all received elements.",
            assert::light
        );
    }

    // error code can be unused if KTHROW is removed at compile time
    [[maybe_unused]] int err = MPI_Gatherv(
        send_buf.data(),                 // send buffer
        send_count.get_single_element(), // send count
        send_type.get_single_element(),  // send type
        recv_buf.data(),                 // recv buffer
        recv_counts.data(),              // recv counts
        recv_displs.data(),              // recv displacements
        recv_type.get_single_element(),  // recv type
        root.rank_signed(),              // root rank
        this->mpi_communicator()         // communicator
    );
    this->mpi_error_hook(err, "MPI_Gather");
    return make_mpi_result<std::tuple<Args...>>(
        std::move(recv_buf),
        std::move(recv_counts),
        std::move(recv_displs),
        std::move(send_count),
        std::move(send_type),
        std::move(recv_type)
    );
}
/// @}
