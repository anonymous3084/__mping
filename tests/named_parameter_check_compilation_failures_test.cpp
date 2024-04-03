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

#include <vector>

#include "xxxxx-1ing/named_parameters.hpp"
#include "named_parameter_check_common.hpp"

int main(int /* argc */, char** /* argv */) {
#if defined(MISSING_REQUIRED_PARAMETER)
    testing::test_required_send_buf();
#elif defined(UNSUPPORTED_PARAMETER_NO_PARAMETERS)
    std::vector<int> v;
    testing::test_empty_arguments(xxxxx-1ing::send_buf(v));
#elif defined(UNSUPPORTED_PARAMETER_ONLY_OPTIONAL_PARAMETERS)
    std::vector<int> v;
    testing::test_optional_recv_buf(xxxxx-1ing::send_buf(v));
#elif defined(DUPLICATE_PARAMETERS)
    std::vector<int> v;
    testing::test_required_send_buf(xxxxx-1ing::send_buf(v), xxxxx-1ing::send_buf(v));
#else
    // If none of the above sections is active, this file will compile successfully.
#endif
}
