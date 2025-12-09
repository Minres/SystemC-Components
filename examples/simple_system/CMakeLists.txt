cmake_minimum_required(VERSION 3.20)
project (simple_system)

find_package(Boost COMPONENTS program_options REQUIRED QUIET)

add_executable (${PROJECT_NAME} 
    plic.cpp
    uart.cpp
    spi.cpp
    gpio.cpp
    test_initiator.cpp
    simple_system.cpp
    sc_main.cpp
)
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC scc)
target_link_libraries(${PROJECT_NAME} PUBLIC Boost::program_options)
if(APPLE)
    set_target_properties (${PROJECT_NAME} PROPERTIES LINK_FLAGS
        -Wl,-U,_sc_main,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif()

add_test(NAME simple_system_example COMMAND ${PROJECT_NAME} -t)
