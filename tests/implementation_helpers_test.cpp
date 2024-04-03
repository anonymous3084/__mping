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

#include <gmock/gmock.h>
#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/implementation_helpers.hpp"
#include "xxxxx-1ing/named_parameters.hpp"
#include "xxxxx-1ing/parameter_objects.hpp"

using namespace ::testing;
using namespace ::xxxxx-1ing;

TEST(ImplementationHelpersTest, is_valid_rank_in_comm) {
    xxxxx-1ing::Communicator comm;

    auto valid_value_rank_parameter = source(0);
    EXPECT_TRUE(xxxxx-1ing::internal::is_valid_rank_in_comm(valid_value_rank_parameter, comm, false, false));
    EXPECT_TRUE(xxxxx-1ing::internal::is_valid_rank_in_comm(valid_value_rank_parameter, comm, true, false));
    EXPECT_TRUE(xxxxx-1ing::internal::is_valid_rank_in_comm(valid_value_rank_parameter, comm, false, true));
    EXPECT_TRUE(xxxxx-1ing::internal::is_valid_rank_in_comm(valid_value_rank_parameter, comm, true, true));

    auto invalid_value_rank_parameter = source(comm.size());
    EXPECT_FALSE(xxxxx-1ing::internal::is_valid_rank_in_comm(invalid_value_rank_parameter, comm, false, false));
    EXPECT_FALSE(xxxxx-1ing::internal::is_valid_rank_in_comm(invalid_value_rank_parameter, comm, true, false));
    EXPECT_FALSE(xxxxx-1ing::internal::is_valid_rank_in_comm(invalid_value_rank_parameter, comm, false, true));
    EXPECT_FALSE(xxxxx-1ing::internal::is_valid_rank_in_comm(invalid_value_rank_parameter, comm, true, true));

    auto null_rank_parameter = source(rank::null);
    EXPECT_FALSE(xxxxx-1ing::internal::is_valid_rank_in_comm(null_rank_parameter, comm, false, false));
    EXPECT_TRUE(xxxxx-1ing::internal::is_valid_rank_in_comm(null_rank_parameter, comm, true, false));
    EXPECT_FALSE(xxxxx-1ing::internal::is_valid_rank_in_comm(null_rank_parameter, comm, false, true));
    EXPECT_TRUE(xxxxx-1ing::internal::is_valid_rank_in_comm(null_rank_parameter, comm, true, true));

    auto any_rank_parameter = source(rank::any);
    EXPECT_FALSE(xxxxx-1ing::internal::is_valid_rank_in_comm(any_rank_parameter, comm, false, false));
    EXPECT_FALSE(xxxxx-1ing::internal::is_valid_rank_in_comm(any_rank_parameter, comm, true, false));
    EXPECT_TRUE(xxxxx-1ing::internal::is_valid_rank_in_comm(any_rank_parameter, comm, false, true));
    EXPECT_TRUE(xxxxx-1ing::internal::is_valid_rank_in_comm(any_rank_parameter, comm, true, true));
}
