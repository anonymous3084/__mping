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

#include "xxxxx-1ing/assertion_levels.hpp"
#undef KASSERT_ASSERTION_LEVEL
#define KASSERT_ASSERTION_LEVEL xxxxx-1_ASSERTION_LEVEL_HEAVY_COMMUNICATION

#include <set>

#include <kassert/kassert.hpp>
#include <mpi.h>

#include "xxxxx-1ing/environment.hpp"
using namespace ::xxxxx-1ing;

// This is not using google test because our test setup would call MPI_Init before running any tests
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    KASSERT(!mpi_env.initialized());
    KASSERT(!mpi_env.finalized());

#ifndef xxxxx-1_ENVIRONMENT_TEST_INIT_FINALIZE_NECESSARY
    MPI_Init(&argc, &argv);
#endif

    {
#if defined(xxxxx-1_ENVIRONMENT_TEST_NO_PARAM)
        Environment<InitMPIMode::InitFinalizeIfNecessary> environment;
#elif defined(xxxxx-1_ENVIRONMENT_TEST_WITH_PARAM)
        Environment<InitMPIMode::InitFinalizeIfNecessary> environment(argc, argv);
#else
        static_assert(false, "Define either xxxxx-1_ENVIRONMENT_TEST_NO_PARAM or xxxxx-1_ENVIRONMENT_TEST_WITH_PARAM");
#endif

        KASSERT(environment.initialized());
        KASSERT(!environment.finalized());

#ifndef xxxxx-1_ENVIRONMENT_TEST_INIT_FINALIZE_NECESSARY
        MPI_Finalize();
#endif
    }

    KASSERT(mpi_env.finalized());
    return 0;
}
