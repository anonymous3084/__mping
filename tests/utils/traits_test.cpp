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

#include <list>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <xxxxx-1ing/utils/traits.hpp>

TEST(TraitsTest, is_range) {
    EXPECT_TRUE(xxxxx-1ing::is_range_v<std::vector<int>>);
    EXPECT_FALSE(xxxxx-1ing::is_range_v<int>);
}

TEST(TraitsTest, is_contiguous_sized_range) {
    EXPECT_TRUE(xxxxx-1ing::is_contiguous_sized_range_v<std::vector<int>>);
    EXPECT_FALSE(xxxxx-1ing::is_contiguous_sized_range_v<std::list<int>>);
    EXPECT_FALSE(xxxxx-1ing::is_contiguous_sized_range_v<int>);
}

TEST(TraitsTest, is_pair_like) {
    EXPECT_TRUE((xxxxx-1ing::is_pair_like_v<std::pair<int, int>>));
    EXPECT_TRUE((xxxxx-1ing::is_pair_like_v<std::tuple<int, int>>));
    EXPECT_FALSE((xxxxx-1ing::is_pair_like_v<std::tuple<int, int, int>>));
    EXPECT_FALSE(xxxxx-1ing::is_pair_like_v<int>);
}

TEST(TraitsTest, is_destination_buffer_pair) {
    EXPECT_TRUE((xxxxx-1ing::is_destination_buffer_pair_v<std::pair<int, std::vector<int>>>));
    EXPECT_FALSE((xxxxx-1ing::is_destination_buffer_pair_v<std::vector<int>>));
    EXPECT_FALSE((xxxxx-1ing::is_destination_buffer_pair_v<std::pair<std::string, std::vector<int>>>));
}

TEST(TraitsTest, is_sparse_send_buffer) {
    EXPECT_TRUE((xxxxx-1ing::is_sparse_send_buffer_v<std::unordered_map<int, std::vector<int>>>));
    EXPECT_TRUE((xxxxx-1ing::is_sparse_send_buffer_v<std::vector<std::pair<int, std::vector<int>>>>));
    EXPECT_FALSE(xxxxx-1ing::is_sparse_send_buffer_v<std::vector<int>>);
    EXPECT_FALSE(xxxxx-1ing::is_sparse_send_buffer_v<std::vector<std::vector<int>>>);
}
