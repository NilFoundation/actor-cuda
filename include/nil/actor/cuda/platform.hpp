//---------------------------------------------------------------------------//
// Copyright (c) 2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

#include <nil/actor/ref_counted.hpp>

#include <nil/actor/cuda/device.hpp>

namespace nil {
    namespace actor {
        namespace cuda {

            class platform;

            using platform_ptr = intrusive_ptr<platform>;

            class platform : public ref_counted {
            public:
                friend class program;

                template<class T, class... Ts>
                friend intrusive_ptr<T> nil::actor::make_counted(Ts &&...);

                inline const std::vector<device_ptr> &devices() const;

                inline const std::string &name() const;

                inline const std::string &vendor() const;

                inline const std::string &version() const;

                static platform_ptr create(cl_platform_id platform_id, unsigned start_id);

            private:
                platform(cl_platform_id platform_id, detail::raw_context_ptr context, std::string name,
                         std::string vendor, std::string version, std::vector<device_ptr> devices);

                ~platform();

                static std::string platform_info(cl_platform_id platform_id, unsigned info_flag);

                cl_platform_id platform_id_;
                detail::raw_context_ptr context_;
                std::string name_;
                std::string vendor_;
                std::string version_;
                std::vector<device_ptr> devices_;
            };

            /******************************************************************************\
             *                 implementation of inline member functions                  *
            \******************************************************************************/

            inline const std::vector<device_ptr> &platform::devices() const {
                return devices_;
            }

            inline const std::string &platform::name() const {
                return name_;
            }

            inline const std::string &platform::vendor() const {
                return vendor_;
            }

            inline const std::string &platform::version() const {
                return version_;
            }

        }    // namespace cuda
    }        // namespace actor
}    // namespace nil
