// This file is part of xxxxx-1.
//
// Copyright 2024 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version. xxxxx-1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with xxxxx-1.  If not, see
// <https://www.gnu.org/licenses/>.

#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/p2p/irecv.hpp"
#include "xxxxx-1ing/p2p/isend.hpp"
#include "xxxxx-1ing/request_pool.hpp"

int main() {
    using namespace xxxxx-1ing;
    Environment  env;
    Communicator comm;
    RequestPool  pool;
    if (comm.rank() == 0) {
        for (int i = 0; i < comm.size(); ++i) {
            comm.isend(send_buf(i), destination(i), tag(i), request(pool.get_request()));
        }
    }
    int val;
    comm.irecv(recv_buf(val), request(pool.get_request()));
    auto statuses = pool.wait_all(statuses_out());
    for (MPI_Status& native_status: statuses) {
        Status status(native_status);
        std::cout << "[R" << comm.rank() << "] "
                  << "Status(source="
                  << (status.source_signed() == MPI_PROC_NULL ? "MPI_PROC_NULL" : std::to_string(status.source_signed())
                     )
                  << ", tag=" << (status.tag() == MPI_ANY_TAG ? "MPI_ANY_TAG" : std::to_string(status.tag()))
                  << ", count=" << status.count<int>() << ")" << std::endl;
    }
}
