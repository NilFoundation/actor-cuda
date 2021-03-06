#include <fstream>

#include <nil/actor/detail/type_list.hpp>
#include <nil/actor/raise_error.hpp>

#include <nil/actor/cuda/device.hpp>
#include <nil/actor/cuda/manager.hpp>
#include <nil/actor/cuda/platform.hpp>
#include <nil/actor/cuda/opencl_error.hpp>

namespace nil {
    namespace actor {
        namespace cuda {

            optional<device_ptr> manager::find_device(std::size_t dev_id) const {
                if (platforms_.empty()) {
                    return none;
                }
                std::size_t to = 0;
                for (auto &pl : platforms_) {
                    auto from = to;
                    to += pl->devices().size();
                    if (dev_id >= from && dev_id < to) {
                        return pl->devices()[dev_id - from];
                    }
                }
                return none;
            }

            void manager::init(spawner_config &) {
                // get number of available platforms
                auto num_platforms = v1get<cl_uint>(ACTOR_CLF(clGetPlatformIDs));
                // get platform ids
                std::vector<cl_platform_id> platform_ids(num_platforms);
                v2callcl(ACTOR_CLF(clGetPlatformIDs), num_platforms, platform_ids.data());
                if (platform_ids.empty())
                    ACTOR_RAISE_ERROR("no OpenCL platform found");
                // initialize platforms (device discovery)
                unsigned current_device_id = 0;
                for (auto &pl_id : platform_ids) {
                    platforms_.push_back(platform::create(pl_id, current_device_id));
                    current_device_id += static_cast<unsigned>(platforms_.back()->devices().size());
                }
            }

            void manager::start() {
                // nop
            }

            void manager::stop() {
                // nop
            }

            spawner_module::id_t manager::id() const {
                return spawner_module::opencl_manager;
            }

            void *manager::subtype_ptr() {
                return this;
            }

            spawner_module *manager::make(spawner &sys, nil::actor::detail::type_list<>) {
                return new manager {sys};
            }

            program_ptr manager::create_program_from_file(const char *path, const char *options, uint32_t device_id) {
                std::ifstream read_source {std::string(path), std::ios::in};
                std::string kernel_source;
                if (read_source) {
                    read_source.seekg(0, std::ios::end);
                    kernel_source.resize(static_cast<size_t>(read_source.tellg()));
                    read_source.seekg(0, std::ios::beg);
                    read_source.read(&kernel_source[0], static_cast<std::streamsize>(kernel_source.size()));
                    read_source.close();
                } else {
                    ACTOR_RAISE_ERROR("create_program_from_file: path not found");
                }
                return create_program(kernel_source.c_str(), options, device_id);
            }

            program_ptr manager::create_program(const char *kernel_source, const char *options, uint32_t device_id) {
                auto dev = find_device(device_id);
                if (!dev) {
                    ACTOR_RAISE_ERROR("create_program: no device found");
                }
                return create_program(kernel_source, options, *dev);
            }

            program_ptr manager::create_program_from_file(const char *path, const char *options, const device_ptr dev) {
                std::ifstream read_source {std::string(path), std::ios::in};
                std::string kernel_source;
                if (read_source) {
                    read_source.seekg(0, std::ios::end);
                    kernel_source.resize(static_cast<size_t>(read_source.tellg()));
                    read_source.seekg(0, std::ios::beg);
                    read_source.read(&kernel_source[0], static_cast<std::streamsize>(kernel_source.size()));
                    read_source.close();
                } else {
                    ACTOR_RAISE_ERROR("create_program_from_file: path not found");
                }
                return create_program(kernel_source.c_str(), options, dev);
            }

            program_ptr manager::create_program(const char *kernel_source, const char *options, const device_ptr dev) {
                // create program object from kernel source
                std::size_t kernel_source_length = strlen(kernel_source);
                detail::raw_program_ptr pptr;
                pptr.reset(v2get(ACTOR_CLF(clCreateProgramWithSource), dev->context_.get(), 1u, &kernel_source,
                                 &kernel_source_length),
                           false);
                // build program from program object
                auto dev_tmp = dev->device_id_.get();
                auto err = clBuildProgram(pptr.get(), 1, &dev_tmp, options, nullptr, nullptr);
                if (err != CL_SUCCESS) {
                    if (err == CL_BUILD_PROGRAM_FAILURE) {
                        std::size_t buildlog_buffer_size = 0;
                        // get the log length
                        clGetProgramBuildInfo(pptr.get(), dev_tmp, CL_PROGRAM_BUILD_LOG, 0, nullptr,
                                              &buildlog_buffer_size);
                        std::vector<char> buffer(buildlog_buffer_size);
                        // fill the buffer with buildlog informations
                        clGetProgramBuildInfo(pptr.get(), dev_tmp, CL_PROGRAM_BUILD_LOG,
                                              sizeof(char) * buildlog_buffer_size, buffer.data(), nullptr);
                        std::ostringstream ss;
                        ss << "############## Build log ##############" << std::endl
                           << std::string(buffer.data()) << std::endl
                           << "#######################################";
                        // seems that just apple implemented the
                        // pfn_notify callback, but we can get
                        // the build log
#ifndef BOOST_OS_MACOS_AVAILABLE
                        ACTOR_LOG_ERROR(ACTOR_ARG(ss.str()));
#endif
                    }
                    ACTOR_RAISE_ERROR("clBuildProgram failed");
                }
                cl_uint number_of_kernels = 0;
                clCreateKernelsInProgram(pptr.get(), 0u, nullptr, &number_of_kernels);
                std::map<std::string, detail::raw_kernel_ptr> available_kernels;
                if (number_of_kernels > 0) {
                    std::vector<cl_kernel> kernels(number_of_kernels);
                    err = clCreateKernelsInProgram(pptr.get(), number_of_kernels, kernels.data(), nullptr);
                    if (err != CL_SUCCESS)
                        ACTOR_RAISE_ERROR("clCreateKernelsInProgram failed");
                    for (cl_uint i = 0; i < number_of_kernels; ++i) {
                        std::size_t len;
                        clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, 0, nullptr, &len);
                        std::vector<char> name(len);
                        err = clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, len,
                                              reinterpret_cast<void *>(name.data()), nullptr);
                        if (err != CL_SUCCESS)
                            ACTOR_RAISE_ERROR("clGetKernelInfo failed");
                        detail::raw_kernel_ptr kernel;
                        kernel.reset(std::move(kernels[i]));
                        available_kernels.emplace(std::string(name.data()), std::move(kernel));
                    }
                } else {
                    ACTOR_LOG_WARNING(
                        "Could not built all kernels in program. Since this happens"
                        " on some platforms, we'll ignore this and try to build"
                        " each kernel individually by name.");
                }
                return make_counted<program>(dev->context_, dev->queue_, pptr, std::move(available_kernels));
            }

            manager::manager(spawner &sys) : system_(sys) {
                // nop
            }

            manager::~manager() {
                // nop
            }

        }    // namespace cuda
    }        // namespace actor
}    // namespace nil
