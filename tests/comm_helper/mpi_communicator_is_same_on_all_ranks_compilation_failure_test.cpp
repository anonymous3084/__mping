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

#include "xxxxx-1ing/comm_helper/is_same_on_all_ranks.hpp"
#include "xxxxx-1ing/communicator.hpp"

int main(int /*argc*/, char** /*argv*/) {
    using namespace ::xxxxx-1ing;
    using namespace ::xxxxx-1ing::internal;

    [[maybe_unused]] Communicator comm;
    [[maybe_unused]] int          value = 0;

#if defined(VALUE_IS_A_POINTER)
    std::ignore = comm.is_same_on_all_ranks(&value);
#else
// If none of the above sections is active, this file will compile successfully.
#endif
}
