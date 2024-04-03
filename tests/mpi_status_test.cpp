// This file is part of xxxxx-1.
//
// Copyright 2022 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version. xxxxx-1 is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with xxxxx-1.  If not, see <https://www.gnu.org/licenses/>.

#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include "xxxxx-1ing/checking_casts.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/status.hpp"

TEST(StatusTest, basic) {
    xxxxx-1ing::Communicator comm;
    ASSERT_GE(comm.size(), 2);

    // The initial values of a status object are undefined.
    // We therefore have to do something useful with it to populate it.
    // First probing with a native status (expected) and then using a wrapped status (actual) for receiving should
    // result in the same fields in the status object.
    if (comm.is_root()) {
        std::vector<int> v = {1, 2, 3, 4, 5};
        MPI_Send(v.data(), xxxxx-1ing::asserting_cast<int>(v.size()), MPI_INT, 1, 42, comm.mpi_communicator());
    } else if (comm.rank() == 1) {
        MPI_Status expected;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm.mpi_communicator(), &expected);
        ASSERT_EQ(expected.MPI_SOURCE, 0);
        ASSERT_EQ(expected.MPI_TAG, 42);
        int recv_count;
        MPI_Get_count(&expected, MPI_INT, &recv_count);
        ASSERT_EQ(recv_count, 5);
        MPI_Get_count(&expected, MPI_BYTE, &recv_count);
        ASSERT_EQ(recv_count, sizeof(int) * 5);

        xxxxx-1ing::Status  actual;
        std::vector<int> v(5);
        MPI_Recv(
            v.data(),
            xxxxx-1ing::asserting_cast<int>(v.size()),
            MPI_INT,
            MPI_ANY_SOURCE,
            MPI_ANY_TAG,
            comm.mpi_communicator(),
            &actual.native()
        );
        ASSERT_EQ(actual.source(), expected.MPI_SOURCE);
        ASSERT_EQ(actual.source_signed(), expected.MPI_SOURCE);
        ASSERT_EQ(actual.tag(), expected.MPI_TAG);
        ASSERT_EQ(actual.count<int>(), 5);
        ASSERT_EQ(actual.count(MPI_INT), 5u);
        ASSERT_EQ(actual.count<std::byte>(), sizeof(int) * 5u);
        ASSERT_EQ(actual.count(MPI_BYTE), sizeof(int) * 5u);
        ASSERT_EQ(actual.count<int>(), actual.count_signed<int>());
        ASSERT_EQ(actual.count(MPI_INT), actual.count_signed(MPI_INT));
        ASSERT_EQ(actual.count<std::byte>(), actual.count_signed<std::byte>());
        ASSERT_EQ(actual.count(MPI_BYTE), actual.count_signed(MPI_BYTE));

        // now lets wrap the native status and check if that also works
        xxxxx-1ing::Status native_wrapped(expected);
        ASSERT_EQ(native_wrapped.source(), expected.MPI_SOURCE);
        ASSERT_EQ(native_wrapped.source_signed(), expected.MPI_SOURCE);
        ASSERT_EQ(native_wrapped.tag(), expected.MPI_TAG);
        ASSERT_EQ(native_wrapped.count<int>(), 5);
        ASSERT_EQ(native_wrapped.count(MPI_INT), 5);
        ASSERT_EQ(native_wrapped.count<std::byte>(), sizeof(int) * 5);
        ASSERT_EQ(native_wrapped.count(MPI_BYTE), sizeof(int) * 5);
    }
}
