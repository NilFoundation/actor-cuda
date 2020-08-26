include(CMTest)

cm_test_link_libraries(${CMAKE_WORKSPACE_NAME}_cuda)

if(NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
    cm_find_package(Boost REQUIRED COMPONENTS system unit_test_framework)
endif()

if(NOT CUDA_FOUND)
    cm_find_package(CUDA REQUIRED)
endif()

cm_test_link_libraries(${CMAKE_WORKSPACE_NAME}_${CURRENT_PROJECT_NAME}

                       ${CMAKE_WORKSPACE_NAME}::core

                       ${Boost_LIBRARIES}
                       ${CUDA_LIBRARIES})

cm_test(NAME cuda_test SOURCES opencl.cpp)

target_include_directories(cuda_test PRIVATE
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test/include>"
                           "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"

                           ${CUDA_INCLUDE_DIRS}
                           ${Boost_INCLUDE_DIRS})


set_target_properties(cuda_test PROPERTIES
                      CXX_STANDARD 11
                      CXX_STANDARD_REQUIRED TRUE)

get_target_property(target_type Boost::unit_test_framework TYPE)
if(target_type STREQUAL "SHARED_LIB")
    target_compile_definitions(cuda_test PRIVATE BOOST_TEST_DYN_LINK)
elseif(target_type STREQUAL "STATIC_LIB")

endif()