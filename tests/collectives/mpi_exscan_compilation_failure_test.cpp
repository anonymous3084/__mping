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

#include "../helpers_for_testing.hpp"
#include "xxxxx-1ing/collectives/exscan.hpp"
#include "xxxxx-1ing/communicator.hpp"

using namespace ::xxxxx-1ing;
using namespace ::testing;

int main(int /*argc*/, char** /*argv*/) {
    using namespace ::xxxxx-1ing;
    Communicator           comm;
    std::vector<int> const input{1};
    std::vector<int>       recv_buffer(1);

#if defined(SEND_RECV_TYPE_GIVEN_BUT_NO_SEND_RECV_COUNT)
    comm.exscan(send_buf(input), send_recv_type(MPI_INT), op(xxxxx-1ing::ops::plus<>{}), recv_buf<no_resize>(recv_buffer));
#elif defined(SEND_RECV_TYPE_GIVEN_BUT_RESIZE_POLICY_IS_RESIZE_TO_FIT)
    comm.exscan(
        send_buf(input),
        send_recv_type(MPI_INT),
        send_recv_count(1),
        op(xxxxx-1ing::ops::plus<>{}),
        recv_buf<resize_to_fit>(recv_buffer)
    );
#elif defined(SEND_RECV_TYPE_GIVEN_BUT_RESIZE_POLICY_IS_GROW_ONLY)
    comm.exscan(
        send_buf(input),
        send_recv_type(MPI_INT),
        send_recv_count(1),
        op(xxxxx-1ing::ops::plus<>{}),
        recv_buf<grow_only>(recv_buffer)
    );
#elif defined(SINGLE_VARIANT_WITH_VECTOR)
    std::vector<int> input{value};
    int const        result = comm.exscan_single(send_buf(input), op(xxxxx-1ing::ops::plus<>{}));
#else
    // If none of the above sections is active, this file will compile successfully.
    comm.exscan(
        send_buf(input),
        send_recv_type(MPI_INT),
        send_recv_count(1),
        op(xxxxx-1ing::ops::plus<>{}),
        recv_buf<no_resize>(recv_buffer)
    );
#endif
}
