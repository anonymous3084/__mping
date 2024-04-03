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
#include "xxxxx-1ing/collectives/gather.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/environment.hpp"
#include "xxxxx-1ing/named_parameters.hpp"

int main() {
    using namespace xxxxx-1ing;

    xxxxx-1ing::Environment  e;
    xxxxx-1ing::Communicator comm;

    std::vector<int> input(comm.size());
    std::iota(input.begin(), input.end(), 0);

    { // Gather all inputs on rank 0.
        [[maybe_unused]] auto output = comm.gather(send_buf(input));
    }

    { // Receive the gathered data in an existing container on rank 1.
        std::vector<int> output;
        comm.gather(send_buf(input), recv_buf<resize_to_fit>(output), root(0));
    }

    return EXIT_SUCCESS;
}
