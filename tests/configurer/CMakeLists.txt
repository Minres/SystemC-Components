add_executable (configurer sc_main.cpp)
target_link_libraries (configurer LINK_PUBLIC scc-sysc)
add_test(NAME configurer_test COMMAND configurer ${CMAKE_CURRENT_SOURCE_DIR}/test.yaml)