add_executable(example_named_parameter_selection named_parameter_selection_example.cpp)
target_link_libraries(example_named_parameter_selection xxxxx-1ing)
target_compile_options(example_named_parameter_selection PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_reduce reduce_example.cpp)
target_link_libraries(example_reduce xxxxx-1ing)
target_compile_options(example_reduce PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_scatter scatter_example.cpp)
target_link_libraries(example_scatter xxxxx-1ing)
target_compile_options(example_scatter PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_alltoall alltoall_example.cpp)
target_link_libraries(example_alltoall xxxxx-1ing)
target_compile_options(example_alltoall PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_alltoallv alltoallv_example.cpp)
target_link_libraries(example_alltoallv xxxxx-1ing)
target_compile_options(example_alltoallv PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_gather gather_example.cpp)
target_link_libraries(example_gather xxxxx-1ing)
target_compile_options(example_gather PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_gatherv gatherv_example.cpp)
target_link_libraries(example_gatherv xxxxx-1ing)
target_compile_options(example_gatherv PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_allgather allgather_example.cpp)
target_link_libraries(example_allgather xxxxx-1ing)
target_compile_options(example_allgather PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_allgatherv allgatherv_example.cpp)
target_link_libraries(example_allgatherv xxxxx-1ing)
target_compile_options(example_allgatherv PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_bcast bcast_example.cpp)
target_link_libraries(example_bcast xxxxx-1ing)
target_compile_options(example_bcast PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_timer timer_example.cpp)
target_link_libraries(example_timer xxxxx-1ing)
target_compile_options(example_timer PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_request_pool request_pool_example.cpp)
target_link_libraries(example_request_pool xxxxx-1ing)
target_compile_options(example_timer PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_datatype datatype_example.cpp)
target_link_libraries(example_datatype xxxxx-1ing)
target_compile_options(example_datatype PRIVATE ${xxxxx-1_WARNING_FLAGS})

add_executable(example_serialization serialization_example.cpp)
target_link_libraries(example_serialization xxxxx-1ing cereal::cereal)
target_compile_options(example_serialization PRIVATE ${xxxxx-1_WARNING_FLAGS})
