// This file is part of xxxxx-1.
//
// Copyright 2022 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version. xxxxx-1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with xxxxx-1.  If not, see
// <https://www.gnu.org/licenses/>.

#include <iostream>
#include <numeric>
#include <vector>

#include <mpi.h>

#include "helpers_for_examples.hpp"
#include "xxxxx-1ing/checking_casts.hpp"
#include "xxxxx-1ing/collectives/alltoall.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/environment.hpp"
#include "xxxxx-1ing/named_parameters.hpp"

int main() {
    using namespace xxxxx-1ing;

    xxxxx-1ing::Environment e;
    Communicator         comm;

    std::vector<int> input(2u * comm.size(), comm.rank_signed());
    std::vector<int> output;

    { // Basic alltoall example. Automatically deduce the send/recv counts and allocate a receive buffer.
        output = comm.alltoall(send_buf(input));
        print_result_on_root(output, comm);
    }

    print_on_root("------", comm);

    { // Use an existing recv buffer but resize it to fit the received data.
        comm.alltoall(send_buf(input), recv_buf<resize_to_fit>(output));
        print_result_on_root(output, comm);
    }

    print_on_root("------", comm);

    { // When the send and receive counts are known, we can provide them explicitly. Additionally, we can use a single
      // buffer for sending and receiving (inplace).
        comm.alltoall(send_recv_buf(input), send_recv_count(2));
        print_result_on_root(input, comm);
    }

    return 0;
}
