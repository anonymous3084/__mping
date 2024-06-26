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

#include <gtest/gtest.h>

#include "xxxxx-1ing/has_member.hpp"

namespace xxxxx-1ing::type_traits {
xxxxx-1_MAKE_HAS_MEMBER(foo)
} // namespace xxxxx-1ing::type_traits

class ClassWithFoo {
public:
    int foo();
};

class ClassWithFooAndArguments {
public:
    int foo(double, char);
};

class ClassWithFooTemplate {
public:
    template <typename T, typename K>
    int foo();
};

class ClassWithFooTemplateDeducable {
public:
    template <typename T, typename K>
    int foo(T, K);
};

class EmptyClass {};

TEST(HasMemberTest, make_has_member_works) {
    EXPECT_TRUE(xxxxx-1ing::type_traits::has_member_foo<ClassWithFoo>::value);
    EXPECT_TRUE(xxxxx-1ing::type_traits::has_member_foo_v<ClassWithFoo>);

    EXPECT_FALSE(xxxxx-1ing::type_traits::has_member_foo<ClassWithFooAndArguments>::value);
    EXPECT_FALSE(xxxxx-1ing::type_traits::has_member_foo_v<ClassWithFooAndArguments>);

    EXPECT_TRUE((xxxxx-1ing::type_traits::has_member_foo<ClassWithFooAndArguments, double, char>::value));
    EXPECT_TRUE((xxxxx-1ing::type_traits::has_member_foo_v<ClassWithFooAndArguments, double, char>));

    EXPECT_FALSE(xxxxx-1ing::type_traits::has_member_foo<ClassWithFooTemplate>::value);
    EXPECT_FALSE(xxxxx-1ing::type_traits::has_member_foo_v<ClassWithFooTemplate>);

    // instantiate the temple parameters
    EXPECT_TRUE((xxxxx-1ing::type_traits::has_member_foo<ClassWithFooTemplate>::value_with_template_params<double, char>));
    EXPECT_TRUE((xxxxx-1ing::type_traits::has_member_foo<ClassWithFooTemplate>::value_with_template_params<double, char>));
    // providing only one is not enough
    EXPECT_FALSE((xxxxx-1ing::type_traits::has_member_foo<ClassWithFooTemplate>::value_with_template_params<double>));

    // the template parameters can be deduced, but we can still specify them explicitly
    EXPECT_TRUE((xxxxx-1ing::type_traits::has_member_foo<ClassWithFooTemplateDeducable, double, char>::
                     value_with_template_params<double, char>));
    // the template parameters are specified explicitly and to not match the arguments
    EXPECT_FALSE((xxxxx-1ing::type_traits::has_member_foo<ClassWithFooTemplateDeducable, std::string, char>::
                      value_with_template_params<double, char>));
    // deduced from the arguments
    EXPECT_TRUE((xxxxx-1ing::type_traits::has_member_foo<ClassWithFooTemplateDeducable, double, char>::
                     value_with_template_params<>));

    // deduced from the arguments, but we do not explicitly add template instantiations
    EXPECT_TRUE((xxxxx-1ing::type_traits::has_member_foo_v<ClassWithFooTemplateDeducable, int, double>));

    // Pass only one parameter instead of two
    EXPECT_FALSE((xxxxx-1ing::type_traits::has_member_foo<ClassWithFooTemplate, int>::value));
    EXPECT_FALSE((xxxxx-1ing::type_traits::has_member_foo_v<ClassWithFooTemplate, int>));

    EXPECT_FALSE(xxxxx-1ing::type_traits::has_member_foo<EmptyClass>::value);
    EXPECT_FALSE(xxxxx-1ing::type_traits::has_member_foo_v<EmptyClass>);
}
