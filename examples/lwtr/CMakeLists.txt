cmake_minimum_required(VERSION 3.20)
project(lwtr_example)

add_executable(${PROJECT_NAME} lwtr_example.cpp)
target_link_libraries (${PROJECT_NAME} lwtr scc)
if(APPLE)
    set_target_properties (${PROJECT_NAME} PROPERTIES LINK_FLAGS
        -Wl,-U,_sc_main,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif()

add_test(NAME lwtr_example_test COMMAND ${PROJECT_NAME})