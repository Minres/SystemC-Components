cmake_minimum_required(VERSION 3.12)
find_package(Boost COMPONENTS program_options REQUIRED)

add_executable (cci_mem_example 
    periph.cpp
    test_initiator.cpp
    simple_system.cpp
    sc_main.cpp
)
target_link_libraries (cci_mem_example LINK_PUBLIC scc)
target_link_libraries(cci_mem_example PUBLIC Boost::program_options)
add_test(NAME simple_system_test COMMAND cci_mem_example)
