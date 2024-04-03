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

#include <iostream>
#include <numeric>
#include <vector>

#include <mpi.h>

#include "helpers_for_examples.hpp"
#include "xxxxx-1ing/checking_casts.hpp"
#include "xxxxx-1ing/collectives/allgather.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/environment.hpp"
#include "xxxxx-1ing/named_parameters.hpp"

int main() {
    using namespace xxxxx-1ing;
    xxxxx-1ing::Environment  e;
    xxxxx-1ing::Communicator comm;
    std::vector<int>      input(comm.size(), comm.rank_signed());

    { // Basic form: Provide a send buffer and let xxxxx-1 allocate the receive buffer.
        auto output = comm.allgather(send_buf(input));
        print_result_on_root(output, comm);
    }

    print_on_root("------", comm);

    { // We can also send only parts of the input and specify an explicit receive buffer.
        std::vector<int> output;

        // this can also be achieved with `xxxxx-1ing::Span`
        comm.allgather(send_buf(Span(input.begin(), 2)), recv_buf<resize_to_fit>(output));
        print_result_on_root(output, comm);
        return 0;
    }
}
