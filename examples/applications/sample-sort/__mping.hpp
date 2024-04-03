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
#pragma once
#include <random>

#include <xxxxx-1ing/collectives/allgather.hpp>
#include <xxxxx-1ing/collectives/alltoall.hpp>
#include <xxxxx-1ing/communicator.hpp>
#include <xxxxx-1ing/utils/flatten.hpp>

#include "./common.hpp"
namespace xxxxx-1ing {
template <typename T>
void sort(MPI_Comm comm_, std::vector<T>& data, seed_type seed) {
    Communicator<> comm(comm_);
    size_t const   oversampling_ratio = 16 * static_cast<size_t>(std::log2(comm.size())) + 1;
    std::vector<T> local_samples(oversampling_ratio);
    std::sample(data.begin(), data.end(), local_samples.begin(), oversampling_ratio, std::mt19937{seed});
    auto global_samples = comm.allgather(send_buf(local_samples));
    pick_splitters(comm.size() - 1, oversampling_ratio, global_samples);
    auto buckets = build_buckets(data, global_samples);
    data = with_flattened(buckets).call([&](auto... flattened) { return comm.alltoallv(std::move(flattened)...); });
    std::sort(data.begin(), data.end());
}
} // namespace xxxxx-1ing
