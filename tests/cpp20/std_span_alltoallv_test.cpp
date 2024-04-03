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

#include <algorithm>
#include <numeric>
#include <span>

#include <gtest/gtest.h>

#include "../helpers_for_testing.hpp"
#include "xxxxx-1ing/collectives/alltoall.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/named_parameters.hpp"

using namespace ::xxxxx-1ing;
using namespace ::testing;

TEST(CPP20Tests, alltoall_std_span) {
    Communicator comm;

    std::vector<int> input_vec(comm.size());
    std::iota(input_vec.begin(), input_vec.end(), 0);

    auto result = comm.alltoall(send_buf(std::span<int>(input_vec)));

    EXPECT_EQ(result.size(), comm.size());

    std::vector<int> expected_result(comm.size(), comm.rank_signed());
    EXPECT_EQ(result, expected_result);
}
