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

#include <algorithm>

#include <gtest/gtest.h>
#include <mpi.h>

#include "xxxxx-1ing/checking_casts.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/has_member.hpp"
#include "xxxxx-1ing/named_parameters.hpp"
#include "xxxxx-1ing/p2p/probe.hpp"

using namespace ::xxxxx-1ing;

xxxxx-1_MAKE_HAS_MEMBER(extract_status)

class ProbeTest : public ::testing::Test {
    void SetUp() override {
        // this makes sure that messages don't spill from other tests
        MPI_Barrier(MPI_COMM_WORLD);
    }
    void TearDown() override {
        // this makes sure that messages don't spill to other tests
        MPI_Barrier(MPI_COMM_WORLD);
    }
};

TEST_F(ProbeTest, direct_probe) {
    Communicator     comm;
    std::vector<int> v(comm.rank(), 42);
    MPI_Request      req;
    // Each rank sends a message with its rank as tag to rank 0.
    // The message has comm.rank() elements.
    MPI_Issend(
        v.data(),                      // send_buf
        asserting_cast<int>(v.size()), // send_count
        MPI_INT,                       // send_type
        0,                             // destination
        comm.rank_signed(),            // tag
        comm.mpi_communicator(),       // comm
        &req                           // request
    );
    if (comm.rank() == 0) {
        for (size_t other = 0; other < comm.size(); other++) {
            {
                // return status
                auto result = comm.probe(source(other), tag(asserting_cast<int>(other)), status_out());
                EXPECT_TRUE(has_member_extract_status_v<decltype(result)>);
                auto status = result.extract_status();
                EXPECT_EQ(status.source(), other);
                EXPECT_EQ(status.tag(), other);
                EXPECT_EQ(status.count<int>(), other);
            }
            {
                // wrapped status
                Status kmp_status;
                auto   result = comm.probe(source(other), tag(asserting_cast<int>(other)), status_out(kmp_status));
                EXPECT_FALSE(has_member_extract_status_v<decltype(result)>);
                EXPECT_EQ(kmp_status.source(), other);
                EXPECT_EQ(kmp_status.tag(), other);
                EXPECT_EQ(kmp_status.count<int>(), other);
            }
            {
                // native status
                MPI_Status mpi_status;
                auto       result = comm.probe(source(other), tag(asserting_cast<int>(other)), status_out(mpi_status));
                EXPECT_FALSE(has_member_extract_status_v<decltype(result)>);
                EXPECT_EQ(mpi_status.MPI_SOURCE, other);
                EXPECT_EQ(mpi_status.MPI_TAG, other);
                int count;
                MPI_Get_count(&mpi_status, MPI_INT, &count);
                EXPECT_EQ(count, other);
            }
            {
                // ignore status
                {
                    auto result = comm.probe(source(other), tag(asserting_cast<int>(other)));
                    EXPECT_FALSE(has_member_extract_status_v<decltype(result)>);
                }
                {
                    auto result = comm.probe(source(other), tag(asserting_cast<int>(other)), status(xxxxx-1ing::ignore<>));
                    EXPECT_FALSE(has_member_extract_status_v<decltype(result)>);
                }
            }
            std::vector<int> recv_buf(other);
            MPI_Recv(
                recv_buf.data(),            // recv_buf
                asserting_cast<int>(other), // recv_size
                MPI_INT,                    // recv_type
                asserting_cast<int>(other), // source
                asserting_cast<int>(other), // tag
                MPI_COMM_WORLD,             // comm
                MPI_STATUS_IGNORE           // status
            );
        }
    }
    // ensure that we have received all inflight messages
    MPI_Wait(&req, MPI_STATUS_IGNORE);
}

TEST_F(ProbeTest, any_source_probe) {
    Communicator     comm;
    std::vector<int> v(comm.rank(), 42);
    MPI_Request      req;
    // Each rank sends a message with its rank as tag to rank 0.
    // The message has comm.rank() elements.
    MPI_Issend(
        v.data(),                      // send_buf
        asserting_cast<int>(v.size()), // send_count
        MPI_INT,                       // send_type
        0,                             // destination
        comm.rank_signed(),            // tag
        comm.mpi_communicator(),       // comm
        &req                           // request
    );
    if (comm.rank() == 0) {
        for (size_t other = 0; other < comm.size(); other++) {
            {
                // explicit any source probe
                auto status =
                    comm.probe(source(rank::any), tag(asserting_cast<int>(other)), status_out()).extract_status();
                EXPECT_EQ(status.source(), other);
                EXPECT_EQ(status.tag(), other);
                EXPECT_EQ(status.count<int>(), other);
            }
            {
                // implicit any source probe
                auto status = comm.probe(tag(asserting_cast<int>(other)), status_out()).extract_status();
                EXPECT_EQ(status.source(), other);
                EXPECT_EQ(status.tag(), other);
                EXPECT_EQ(status.count<int>(), other);
            }
            std::vector<int> recv_buf(other);
            MPI_Recv(
                recv_buf.data(),            // recv_buf
                asserting_cast<int>(other), // recv_size
                MPI_INT,                    // recv_type
                asserting_cast<int>(other), // source
                asserting_cast<int>(other), // tag
                MPI_COMM_WORLD,             // comm
                MPI_STATUS_IGNORE           // status
            );
        }
    }
    // ensure that we have received all inflight messages
    MPI_Wait(&req, MPI_STATUS_IGNORE);
}

TEST_F(ProbeTest, any_tag_probe) {
    Communicator     comm;
    std::vector<int> v(comm.rank(), 42);
    MPI_Request      req;

    // Each rank sends a message with its rank as tag to rank 0.
    // The message has comm.rank() elements.
    MPI_Issend(
        v.data(),                      // send_buf
        asserting_cast<int>(v.size()), // send_count
        MPI_INT,                       // send_type
        0,                             // destination
        comm.rank_signed(),            // tag
        comm.mpi_communicator(),       // comm
        &req                           // request
    );
    if (comm.rank() == 0) {
        for (size_t other = 0; other < comm.size(); other++) {
            {
                // explicit any tag probe
                auto status = comm.probe(source(other), tag(tags::any), status_out()).extract_status();
                EXPECT_EQ(status.source(), other);
                EXPECT_EQ(status.tag(), other);
                EXPECT_EQ(status.count<int>(), other);
            }
            {
                // implicit any tag probe
                auto status = comm.probe(source(other), status_out()).extract_status();
                EXPECT_EQ(status.source(), other);
                EXPECT_EQ(status.tag(), other);
                EXPECT_EQ(status.count<int>(), other);
            }
            std::vector<int> recv_buf(other);
            MPI_Recv(
                recv_buf.data(),            // recv_buf
                asserting_cast<int>(other), // recv_size
                MPI_INT,                    // recv_type
                asserting_cast<int>(other), // source
                asserting_cast<int>(other), // tag
                MPI_COMM_WORLD,             // comm
                MPI_STATUS_IGNORE           // status
            );
        }
    }
    // ensure that we have received all inflight messages
    MPI_Wait(&req, MPI_STATUS_IGNORE);
}

TEST_F(ProbeTest, arbitrary_probe_explicit) {
    Communicator     comm;
    std::vector<int> v(comm.rank(), 42);
    MPI_Request      req;

    // Each rank sends a message with its rank as tag to rank 0.
    // The message has comm.rank() elements.
    MPI_Issend(
        v.data(),                      // send_buf
        asserting_cast<int>(v.size()), // send_count
        MPI_INT,                       // send_type
        0,                             // destination
        comm.rank_signed(),            // tag
        comm.mpi_communicator(),       // comm
        &req                           // request
    );
    if (comm.rank() == 0) {
        // because we may receive arbitrary message, we keep track of them
        std::vector<bool> received_message_from(comm.size());

        for (size_t other = 0; other < comm.size(); other++) {
            auto status = comm.probe(source(rank::any), tag(tags::any), status_out()).extract_status();
            auto source = status.source();
            EXPECT_FALSE(received_message_from[source]);
            EXPECT_EQ(status.tag(), status.source_signed());
            EXPECT_EQ(status.count_signed<int>(), source);

            std::vector<int> recv_buf(source);
            MPI_Recv(
                recv_buf.data(),             // recv_buf
                asserting_cast<int>(source), // recv_size
                MPI_INT,                     // recv_type
                asserting_cast<int>(source), // source
                asserting_cast<int>(source), // tag
                MPI_COMM_WORLD,              // comm
                MPI_STATUS_IGNORE            // status
            );
            received_message_from[source] = true;
        }
        // check that we probed all messages
        EXPECT_TRUE(std::all_of(received_message_from.begin(), received_message_from.end(), [](bool const& received) {
            return received;
        }));
    }
    // ensure that we have received all inflight messages
    MPI_Wait(&req, MPI_STATUS_IGNORE);
}

TEST_F(ProbeTest, arbitrary_probe_implicit) {
    Communicator     comm;
    std::vector<int> v(comm.rank(), 42);
    MPI_Request      req;

    // Each rank sends a message with its rank as tag to rank 0.
    // The message has comm.rank() elements.
    MPI_Issend(
        v.data(),                      // send_buf
        asserting_cast<int>(v.size()), // send_count
        MPI_INT,                       // send_type
        0,                             // destination
        comm.rank_signed(),            // tag
        comm.mpi_communicator(),       // comm
        &req                           // request
    );
    if (comm.rank() == 0) {
        // because we may receive arbitrary message, we keep track of them
        std::vector<bool> received_message_from(comm.size());

        for (size_t other = 0; other < comm.size(); other++) {
            auto status = comm.probe(status_out()).extract_status();
            auto source = status.source();
            EXPECT_FALSE(received_message_from[source]);
            EXPECT_EQ(status.tag(), status.source_signed());
            EXPECT_EQ(status.count_signed<int>(), source);

            std::vector<int> recv_buf(source);
            MPI_Recv(
                recv_buf.data(),             // recv_buf
                asserting_cast<int>(source), // recv_size
                MPI_INT,                     // recv_type
                asserting_cast<int>(source), // source
                asserting_cast<int>(source), // tag
                MPI_COMM_WORLD,              // comm
                MPI_STATUS_IGNORE            // status
            );
            received_message_from[source] = true;
        }
        // check that we probed all messages
        EXPECT_TRUE(std::all_of(received_message_from.begin(), received_message_from.end(), [](bool const& received) {
            return received;
        }));
    }
    // ensure that we have received all inflight messages
    MPI_Wait(&req, MPI_STATUS_IGNORE);
}

TEST_F(ProbeTest, probe_null) {
    Communicator comm;
    auto         status = comm.probe(source(rank::null), status_out()).extract_status();
    EXPECT_EQ(status.source_signed(), MPI_PROC_NULL);
    EXPECT_EQ(status.tag(), MPI_ANY_TAG);
    EXPECT_EQ(status.count<int>(), 0);
}
