set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
find_package(Protobuf REQUIRED)
add_executable(test_serialize
        test_serialize.cpp
        test_tuplet.cpp
        test_alignas.cpp
        test_pragma_pack.cpp
        test_pragma_pack_and_alignas_mix.cpp
        test_pb.cpp
        test_pb_benchmark_struct.cpp
        main.cpp
        )
add_test(NAME test_serialize COMMAND test_serialize)
target_compile_definitions(test_serialize PRIVATE
        STRUCT_PACK_USE_INT16_SIZE
        STRUCT_PACK_ENABLE_UNPORTABLE_TYPE
        )
target_link_libraries(test_serialize PRIVATE
        libstruct_pack
        protobuf::libprotobuf
        doctest
        )
protobuf_generate_cpp(TEST_SERIALIZE_PROTO_SRCS
        TEST_SERIALIZE_PROTO_HDRS
        test_pb.proto
        ../benchmark/benchmark.proto
        )
target_include_directories(test_serialize PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_sources(test_serialize PRIVATE
        ${TEST_SERIALIZE_PROTO_SRCS}
        ${TEST_SERIALIZE_PROTO_HDRS}
        )
message(STATUS "TEST_SERIALIZE_PROTO_SRCS: ${TEST_SERIALIZE_PROTO_SRCS}")
message(STATUS "TEST_SERIALIZE_PROTO_HDRS: ${TEST_SERIALIZE_PROTO_HDRS}")
