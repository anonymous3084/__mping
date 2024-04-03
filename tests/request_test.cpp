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

#include <set>

#include <gtest/gtest.h>
#include <mpi.h>

#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/request.hpp"

TEST(RequestTest, wait_with_status_out) {
    MPI_Request           mpi_send_req, mpi_recv_req;
    int                   msg = 42;
    xxxxx-1ing::Communicator comm;
    MPI_Isend(
        &msg,
        1,
        MPI_INT,
        xxxxx-1ing::asserting_cast<int>(comm.rank_shifted_cyclic(1)),
        3,
        comm.mpi_communicator(),
        &mpi_send_req
    );
    int result;
    MPI_Irecv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm.mpi_communicator(), &mpi_recv_req);
    xxxxx-1ing::Request req{mpi_recv_req};
    auto             status = req.wait(xxxxx-1ing::status_out());
    EXPECT_EQ(status.source(), comm.rank_shifted_cyclic(-1));
    EXPECT_EQ(status.tag(), 3);
}

TEST(RequestTest, wait_with_status_in) {
    MPI_Request           mpi_send_req, mpi_recv_req;
    int                   msg = 42;
    xxxxx-1ing::Communicator comm;
    MPI_Isend(
        &msg,
        1,
        MPI_INT,
        xxxxx-1ing::asserting_cast<int>(comm.rank_shifted_cyclic(1)),
        3,
        comm.mpi_communicator(),
        &mpi_send_req
    );
    int result;
    MPI_Irecv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm.mpi_communicator(), &mpi_recv_req);
    xxxxx-1ing::Request req{mpi_recv_req};
    xxxxx-1ing::Status  status;
    req.wait(xxxxx-1ing::status_out(status));
    EXPECT_EQ(status.source(), comm.rank_shifted_cyclic(-1));
    EXPECT_EQ(status.tag(), 3);
}

TEST(RequestTest, wait_with_native_status_in) {
    MPI_Request           mpi_send_req, mpi_recv_req;
    int                   msg = 42;
    xxxxx-1ing::Communicator comm;
    MPI_Isend(
        &msg,
        1,
        MPI_INT,
        xxxxx-1ing::asserting_cast<int>(comm.rank_shifted_cyclic(1)),
        3,
        comm.mpi_communicator(),
        &mpi_send_req
    );
    int result;
    MPI_Irecv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm.mpi_communicator(), &mpi_recv_req);
    xxxxx-1ing::Request req{mpi_recv_req};
    MPI_Status       status;
    req.wait(xxxxx-1ing::status_out(status));
    EXPECT_EQ(status.MPI_SOURCE, comm.rank_shifted_cyclic(-1));
    EXPECT_EQ(status.MPI_TAG, 3);
}

TEST(RequestTest, test_with_status_out) {
    MPI_Request           mpi_send_req, mpi_recv_req;
    int                   msg = 42;
    xxxxx-1ing::Communicator comm;
    MPI_Isend(
        &msg,
        1,
        MPI_INT,
        xxxxx-1ing::asserting_cast<int>(comm.rank_shifted_cyclic(1)),
        3,
        comm.mpi_communicator(),
        &mpi_send_req
    );
    int result;
    MPI_Irecv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm.mpi_communicator(), &mpi_recv_req);
    xxxxx-1ing::Request req{mpi_recv_req};
    auto             status = req.test(xxxxx-1ing::status_out());
    while (!status) {
        status = req.test(xxxxx-1ing::status_out());
    }
    EXPECT_EQ(status->source(), comm.rank_shifted_cyclic(-1));
    EXPECT_EQ(status->tag(), 3);
}

TEST(RequestTest, test_with_status_in) {
    MPI_Request           mpi_send_req, mpi_recv_req;
    int                   msg = 42;
    xxxxx-1ing::Communicator comm;
    MPI_Isend(
        &msg,
        1,
        MPI_INT,
        xxxxx-1ing::asserting_cast<int>(comm.rank_shifted_cyclic(1)),
        3,
        comm.mpi_communicator(),
        &mpi_send_req
    );
    int result;
    MPI_Irecv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm.mpi_communicator(), &mpi_recv_req);
    xxxxx-1ing::Request req{mpi_recv_req};
    xxxxx-1ing::Status  status;
    while (!req.test(xxxxx-1ing::status_out(status))) {
    }
    EXPECT_EQ(status.source(), comm.rank_shifted_cyclic(-1));
    EXPECT_EQ(status.tag(), 3);
}

TEST(RequestTest, test_with_native_status_in) {
    MPI_Request           mpi_send_req, mpi_recv_req;
    int                   msg = 42;
    xxxxx-1ing::Communicator comm;
    MPI_Isend(
        &msg,
        1,
        MPI_INT,
        xxxxx-1ing::asserting_cast<int>(comm.rank_shifted_cyclic(1)),
        3,
        comm.mpi_communicator(),
        &mpi_send_req
    );
    int result;
    MPI_Irecv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm.mpi_communicator(), &mpi_recv_req);
    xxxxx-1ing::Request req{mpi_recv_req};
    MPI_Status       status;
    while (!req.test(xxxxx-1ing::status_out(status))) {
    }
    EXPECT_EQ(status.MPI_SOURCE, comm.rank_shifted_cyclic(-1));
    EXPECT_EQ(status.MPI_TAG, 3);
}

TEST(RequestTest, test_that_never_succeeds) {
    MPI_Request           mpi_recv_req;
    int                   result;
    xxxxx-1ing::Communicator comm;
    // this receive does not match any send
    MPI_Irecv(&result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, comm.mpi_communicator(), &mpi_recv_req);
    xxxxx-1ing::Request req{mpi_recv_req};
    EXPECT_FALSE(req.test(xxxxx-1ing::status_out()));
    MPI_Cancel(&req.mpi_request());
}
