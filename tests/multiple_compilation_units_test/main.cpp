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

/// @file
/// @brief The main file (and first compilation unit) of a test that checks if compiling and running works correctly
/// when linking two compilation units that both use xxxxx-1

#include <cstddef>
#include <numeric>

#include <gtest/gtest.h>
#include <mpi.h>

// include all collectives so we can catch errors
#include "./gatherer.hpp"
#include "xxxxx-1ing/collectives/alltoall.hpp"
#include "xxxxx-1ing/collectives/barrier.hpp"
#include "xxxxx-1ing/collectives/gather.hpp"
#include "xxxxx-1ing/collectives/reduce.hpp"
#include "xxxxx-1ing/collectives/scatter.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/environment.hpp"

/// @brief The main function for this tests. Gathers the ranks on the root (done in a different compilation unit), calls
/// a barrier and checks the result
TEST(TwoCompilationUnitsTest, main) {
    using namespace xxxxx-1ing;
    EXPECT_TRUE(mpi_env.initialized());

    Communicator comm;

    Gatherer gatherer;
    auto     gathered_data = gatherer.gather(comm.rank_signed());

    comm.barrier();

    if (comm.rank() == 0) {
        std::vector<int> expected_result(static_cast<size_t>(comm.size()));
        std::iota(expected_result.begin(), expected_result.end(), 0);
        EXPECT_EQ(gathered_data, expected_result);
    } else {
        EXPECT_TRUE(gathered_data.empty());
    }
}
