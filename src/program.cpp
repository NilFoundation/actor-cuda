//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>

#include <nil/actor/cuda/manager.hpp>
#include <nil/actor/cuda/program.hpp>
#include <nil/actor/cuda/opencl_error.hpp>

namespace nil {
    namespace actor {
        namespace opencl {

            program::program(detail::raw_context_ptr context, detail::raw_command_queue_ptr queue,
                             detail::raw_program_ptr prog,
                             std::map<std::string, detail::raw_kernel_ptr> available_kernels) :
                context_(std::move(context)),
                program_(std::move(prog)), queue_(std::move(queue)), available_kernels_(std::move(available_kernels)) {
                // nop
            }

            program::~program() {
                // nop
            }
        }    // namespace opencl
    }        // namespace actor
}    // namespace nil
