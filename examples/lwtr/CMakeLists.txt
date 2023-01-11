cmake_minimum_required(VERSION 3.12)

add_executable(lwtr_example lwtr_example.cpp)
target_link_libraries (lwtr_example lwtr scc)

add_test(NAME lwtr_example_test COMMAND lwtr_example)