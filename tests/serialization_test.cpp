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

#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mpi.h>

#include "xxxxx-1ing/collectives/bcast.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/p2p/recv.hpp"
#include "xxxxx-1ing/p2p/send.hpp"
#include "xxxxx-1ing/serialization.hpp"

using namespace xxxxx-1ing;

using dict_type = std::unordered_map<std::string, std::string>;

TEST(SerializationTest, basic) {
    xxxxx-1ing::Communicator comm;
    dict_type             data{{"key1", "value1"}, {"key2", "value2"}};
    if (comm.is_root()) {
        for (size_t dst = 0; dst < comm.size(); dst++) {
            if (comm.is_root(dst)) {
                continue;
            }
            comm.send(xxxxx-1ing::send_buf(as_serialized(data)), destination(dst));
        }
    } else {
        auto recv_data = comm.recv(recv_buf(as_deserializable<dict_type>()));
        EXPECT_EQ(recv_data, data);
    }
}

TEST(SerializationTest, basic_recv_to_ref) {
    xxxxx-1ing::Communicator comm;
    dict_type             data{{"key1", "value1"}, {"key2", "value2"}};
    if (comm.is_root()) {
        for (size_t dst = 0; dst < comm.size(); dst++) {
            if (comm.is_root(dst)) {
                continue;
            }
            comm.send(xxxxx-1ing::send_buf(as_serialized(data)), destination(dst));
        }
    } else {
        dict_type recv_data;
        comm.recv(recv_buf(as_deserializable(recv_data)));
        bool returns_nothing = std::is_same_v<decltype(comm.recv(recv_buf(as_deserializable(recv_data)))), void>;
        EXPECT_TRUE(returns_nothing);
        EXPECT_EQ(recv_data, data);
    }
}

TEST(SerializationTest, basic_recv_move_in_out) {
    xxxxx-1ing::Communicator comm;
    dict_type             data{{"key1", "value1"}, {"key2", "value2"}};
    if (comm.is_root()) {
        for (size_t dst = 0; dst < comm.size(); dst++) {
            if (comm.is_root(dst)) {
                continue;
            }
            comm.send(xxxxx-1ing::send_buf(as_serialized(data)), destination(dst));
        }
    } else {
        dict_type recv_data;
        recv_data = comm.recv(recv_buf(as_deserializable(std::move(recv_data))));
        EXPECT_EQ(recv_data, data);
    }
}

TEST(SerializationTest, no_explicit_non_default_archive) {
    xxxxx-1ing::Communicator comm;
    dict_type             data{{"key1", "value1"}, {"key2", "value2"}};
    if (comm.is_root()) {
        for (size_t dst = 0; dst < comm.size(); dst++) {
            if (comm.is_root(dst)) {
                continue;
            }
            comm.send(xxxxx-1ing::send_buf(as_serialized<cereal::JSONOutputArchive>(data)), destination(dst));
        }
    } else {
        auto recv_data = comm.recv(recv_buf(as_deserializable<dict_type, cereal::JSONInputArchive>()));
        EXPECT_EQ(recv_data, data);
    }
}

TEST(SerializationTest, basic_bcast) {
    xxxxx-1ing::Communicator comm;
    dict_type             data;
    if (comm.is_root()) {
        data = {{"key1", "value1"}, {"key2", "value2"}};
    }
    comm.bcast(send_recv_buf(as_serialized(data)));
    bool returns_nothing = std::is_same_v<decltype(comm.bcast(send_recv_buf(as_serialized(data)))), void>;
    EXPECT_TRUE(returns_nothing);

    auto expected_data = dict_type{{"key1", "value1"}, {"key2", "value2"}};
    EXPECT_EQ(data, expected_data);
}

TEST(SerializationTest, basic_bcast_passthrough) {
    xxxxx-1ing::Communicator comm;
    dict_type             data;
    if (comm.is_root()) {
        data = {{"key1", "value1"}, {"key2", "value2"}};
    }
    data = comm.bcast(send_recv_buf(as_serialized(std::move(data))));

    auto expected_data = dict_type{{"key1", "value1"}, {"key2", "value2"}};
    EXPECT_EQ(data, expected_data);
}

struct Foo {
    double           x;
    std::vector<int> v;
    template <class Archive>
    void serialize(Archive& ar) {
        ar(x, v);
    }
    bool operator==(Foo const& other) const {
        return this->x == other.x && this->v == other.v;
    }
};

TEST(SerializationTest, custom_serialization_functions) {
    xxxxx-1ing::Communicator comm;
    Foo                   data = {3.14, {1, 2, 3}};
    if (comm.is_root()) {
        for (size_t dst = 0; dst < comm.size(); dst++) {
            if (comm.is_root(dst)) {
                continue;
            }
            comm.send(xxxxx-1ing::send_buf(as_serialized(data)), destination(dst));
        }
    } else {
        auto recv_data = comm.recv(recv_buf(as_deserializable<Foo>()));
        EXPECT_EQ(recv_data, data);
    }
}
