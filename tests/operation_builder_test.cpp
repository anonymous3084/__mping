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

#include <gtest/gtest.h>

#include "xxxxx-1ing/operation_builder.hpp"

using namespace ::xxxxx-1ing;
using namespace ::xxxxx-1ing::internal;

TEST(OperationBuilderTest, move_constructor_assignment_operator_is_enabled) {
    // simply test that move ctor and assignment operator can be called.
    OperationBuilder op_builder1(ops::plus<>(), ops::commutative);
    OperationBuilder op_builder2(std::move(op_builder1));
    OperationBuilder op_builder3(ops::plus<>(), ops::commutative);
    op_builder3 = std::move(op_builder2);
}
