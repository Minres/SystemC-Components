project (cxs_channel)

add_executable(${PROJECT_NAME} sc_main.cpp)
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC scc::interfaces scc-sysc)
