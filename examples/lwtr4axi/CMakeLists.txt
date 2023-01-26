project (lwtr4axi_example)

add_executable(lwtr4axi_example sc_main.cpp)
target_include_directories(lwtr4axi_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (lwtr4axi_example PUBLIC scc)
target_link_libraries (lwtr4axi_example LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (lwtr4axi_example LINK_PUBLIC ${CMAKE_DL_LIBS})
