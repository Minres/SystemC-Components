add_executable (sc_fixed_tracing sc_main.cpp)
target_link_libraries (sc_fixed_tracing LINK_PUBLIC scc-sysc)
add_test(NAME sc_fixed_tracing_test COMMAND sc_fixed_tracing )