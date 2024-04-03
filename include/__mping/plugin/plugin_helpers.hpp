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
#pragma once

#include <xxxxx-1ing/named_parameter_filtering.hpp>

namespace xxxxx-1ing::plugin {

/// @brief Helper class for using CRTP for mixins. Which are used to implement xxxxx-1ing plugins.
///
/// Taken from https://www.fluentcpp.com/2017/05/19/crtp-helper/
/// @tparam CommunicatorClass Type of the class we want to add functionality to, i.e. `xxxxx-1ing::Communicator`.
/// @tparam DefaultContainerType Default container type of Communicator class
/// @tparam PluginClass Type of the plugin class template which inherits from \c PluginBase and adds functionality to \c
/// CommunicatorClass.
template <
    typename CommunicatorClass,
    template <typename...>
    typename DefaultContainerType,
    template <typename, template <typename...> typename>
    typename PluginClass>
struct PluginBase {
private:
    /// @return Reference to the underlying Communicator class.
    CommunicatorClass& to_communicator() {
        return static_cast<CommunicatorClass&>(*this);
    }

    /// @return const-reference to the underlying Communicator class.
    CommunicatorClass const& to_communicator() const {
        return static_cast<CommunicatorClass const&>(*this);
    }

    PluginBase() {}                                              ///< private constructor
    friend PluginClass<CommunicatorClass, DefaultContainerType>; // this allows only the class inheriting from \c
                                                                 // PluginBase to access the functions of this class.
};

/// @brief Filter the arguments \tparam Args for which the static member function `discard()` of \tparam Predicate
/// returns true and pack (move) remaining arguments into a tuple.
template <typename Predicate, typename... Args>
auto filter_args_into_tuple(Args&&... args) {
    using namespace xxxxx-1ing::internal;
    using ArgsToKeep = typename FilterOut<Predicate, std::tuple<Args...>>::type;
    return construct_buffer_tuple<ArgsToKeep>(args...);
}

} // namespace xxxxx-1ing::plugin
