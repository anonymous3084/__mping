// This file is part of xxxxx-1.
//
// Copyright 2021-2022 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version. xxxxx-1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with xxxxx-1.  If not, see
// <https://www.gnu.org/licenses/>.

#include "xxxxx-1ing/operation_builder.hpp"

int main(int /*argc*/, char** /*argv*/) {
    using namespace ::xxxxx-1ing;
    using namespace ::xxxxx-1ing::internal;
    OperationBuilder op_builder(ops::plus<>(), ops::commutative);
#if defined(COPY_CONSTRUCT_OP_BUILDER_BUFFER)
    // should not be possible to copy construct a buffer (for performance reasons)
    auto tmp = op_builder;
#elif defined(COPY_ASSIGN_OP_BUILDER_BUFFER)
    // should not be possible to copy assign a buffer (for performance reasons)
    op_builder = op_builder;
#else
// If none of the above sections is active, this file will compile successfully.
#endif
}
