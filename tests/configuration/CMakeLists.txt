add_executable (cci-example 
    sc_main.cpp
)
target_link_libraries (cci-example LINK_PUBLIC scc-sysc)
add_test(NAME cci-example_test COMMAND cci-example)