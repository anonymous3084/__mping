// This file is part of xxxxx-1.
//
// Copyright 2021 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version. xxxxx-1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with xxxxx-1.  If not, see
// <https://www.gnu.org/licenses/>.

#include <gtest/gtest.h>

#include "xxxxx-1ing/mpi_ops.hpp"
#include "xxxxx-1ing/named_parameters.hpp"

using namespace ::xxxxx-1ing;
using namespace ::xxxxx-1ing::internal;

TEST(ParameterFactoriesTest, op_commutativity_tags_work) {
    struct MySum {
        int operator()(int const& a, int const& b) const {
            return a + b;
        }
    };
    {
        auto op_object = op(std::plus<>{});
        auto op        = op_object.build_operation<int>();
        EXPECT_EQ(op.op(), MPI_SUM);
        EXPECT_TRUE(decltype(op)::commutative);
    }
    {
        auto op_object = op(MySum{}, xxxxx-1ing::ops::commutative);
        auto op        = op_object.build_operation<int>();
        EXPECT_NE(op.op(), MPI_SUM);
        EXPECT_TRUE(decltype(op)::commutative);
    }
    {
        auto op_object = op(MySum{}, xxxxx-1ing::ops::non_commutative);
        auto op        = op_object.build_operation<int>();
        EXPECT_NE(op.op(), MPI_SUM);
        EXPECT_FALSE(decltype(op)::commutative);
    }
}
