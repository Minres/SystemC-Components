project (sim_speed)
add_executable(${PROJECT_NAME} sc_main.cpp)
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC scc-sysc)
