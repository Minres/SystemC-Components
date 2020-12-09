cmake_minimum_required(VERSION 3.11)

project (axi_axi_test)

add_executable(axi_axi_test sc_main.cpp)

target_link_libraries (axi_axi_test PUBLIC tlm2-interfaces)
target_link_libraries (axi_axi_test PUBLIC scc)
target_link_libraries (axi_axi_test LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (axi_axi_test LINK_PUBLIC ${CMAKE_DL_LIBS})
