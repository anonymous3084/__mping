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
#include "xxxxx-1ing/collectives/gather.hpp"
#include "xxxxx-1ing/communicator.hpp"

using namespace ::xxxxx-1ing;
using namespace ::testing;

int main(int /*argc*/, char** /*argv*/) {
    using namespace ::xxxxx-1ing;
    Communicator     comm;
    std::vector<int> input{0};
    std::vector<int> receive_counts(comm.size(), 1);
    std::vector<int> receive_buffer(comm.size());

#if defined(SEND_TYPE_GIVEN_BUT_NO_SEND_COUNT)
    comm.gatherv(send_buf(input), send_type(MPI_INT));
#elif defined(RECV_TYPE_GIVEN_BUT_NO_RECV_COUNTS)
    comm.gatherv(send_buf(input), recv_type(MPI_INT), recv_buf<no_resize>(receive_buffer));
#elif defined(RECV_TYPE_GIVEN_BUT_RESIZE_POLICY_IS_RESIZE_TO_FIT)
    comm.gatherv(
        send_buf(input),
        recv_type(MPI_INT),
        recv_counts(receive_counts),
        recv_buf<resize_to_fit>(receive_buffer)
    );
#elif defined(RECV_TYPE_GIVEN_BUT_RESIZE_POLICY_IS_GROW_ONLY)
    comm.gatherv(send_buf(input), recv_type(MPI_INT), recv_counts(receive_counts), recv_buf<grow_only>(receive_buffer));
#else
    // If none of the above sections is active, this file will compile successfully.
    comm.gatherv(
        send_buf(input),
        send_type(MPI_INT),
        send_count(1),
        recv_type(MPI_INT),
        recv_counts(receive_counts),
        recv_buf<no_resize>(receive_buffer)
    );
#endif
}
