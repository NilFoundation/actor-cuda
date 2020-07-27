//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <nil/actor/detail/type_list.hpp>

#include <nil/actor/opencl/command.hpp>

namespace nil {
    namespace actor {
        namespace detail {

            // signature for the function that is applied to output arguments
            template<class List>
            struct output_function_sig;

            template<class... Ts>
            struct output_function_sig<detail::type_list<Ts...>> {
                using type = std::function<message(Ts &...)>;
            };

            // derive signature of the command that handles the kernel execution
            template<class T, class List>
            struct command_sig;

            template<class T, class... Ts>
            struct command_sig<T, detail::type_list<Ts...>> {
                using type = opencl::command<T, Ts...>;
            };

            // derive type for a tuple matching the arguments as mem_refs
            template<class List>
            struct tuple_type_of;

            template<class... Ts>
            struct tuple_type_of<detail::type_list<Ts...>> {
                using type = std::tuple<Ts...>;
            };

        }    // namespace detail
    }        // namespace actor
}    // namespace nil
