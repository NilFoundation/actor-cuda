//---------------------------------------------------------------------------//
// Copyright (c) 2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <map>
#include <memory>

#include <nil/actor/ref_counted.hpp>

#include <nil/actor/detail/raw_ptr.hpp>

#include <nil/actor/cuda/device.hpp>
#include <nil/actor/cuda/global.hpp>

namespace nil {
    namespace actor {
        namespace cuda {

            class program;

            using program_ptr = intrusive_ptr<program>;

            /// @brief A wrapper for OpenCL's cl_program.
            class program : public ref_counted {
            public:
                friend class manager;

                template<bool PassConfig, class... Ts>
                friend class actor_facade;

                template<class T, class... Ts>
                friend intrusive_ptr<T> nil::actor::make_counted(Ts &&...);

            private:
                program(detail::raw_context_ptr context, detail::raw_command_queue_ptr queue,
                        detail::raw_program_ptr prog, std::map<std::string, detail::raw_kernel_ptr> available_kernels);

                ~program();

                detail::raw_context_ptr context_;
                detail::raw_program_ptr program_;
                detail::raw_command_queue_ptr queue_;
                std::map<std::string, detail::raw_kernel_ptr> available_kernels_;
            };

        }    // namespace cuda
    }        // namespace actor
}    // namespace nil
