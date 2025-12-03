find_package(GTest REQUIRED)

add_executable(memory_boundary_test
    memory_boundary_test.cpp
    gtest_sc_main.cpp
)

target_link_libraries(memory_boundary_test
    PRIVATE
        GTest::gtest
        scc::scc
        SystemC::systemc
)
target_compile_definitions(memory_boundary_test PRIVATE SC_DISABLE_API_VERSION_CHECK)

add_test(NAME memory_boundary_test COMMAND memory_boundary_test)
