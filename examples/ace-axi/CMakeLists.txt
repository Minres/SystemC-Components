project (ace_axi_example)

add_executable(ace_axi_example sc_main.cpp)
target_include_directories(ace_axi_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (ace_axi_example PUBLIC tlm-interfaces)
target_link_libraries (ace_axi_example PUBLIC scc)
target_link_libraries (ace_axi_example LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (ace_axi_example LINK_PUBLIC ${CMAKE_DL_LIBS})
