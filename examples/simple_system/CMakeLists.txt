cmake_minimum_required(VERSION 3.12)
find_package(Boost COMPONENTS program_options REQUIRED)
add_executable (simple_system 
    plic.cpp
    uart.cpp
    spi.cpp
    gpio.cpp
    test_initiator.cpp
    simple_system.cpp
    sc_main.cpp
)
target_link_libraries (simple_system LINK_PUBLIC scc)
if(TARGET Boost::program_options)
	target_link_libraries(simple_system PUBLIC Boost::program_options)
else()
	target_link_libraries(simple_system PUBLIC ${BOOST_program_options_LIBRARY})
endif()
add_test(NAME simple_system_test COMMAND simple_system)