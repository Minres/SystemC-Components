cmake_minimum_required(VERSION 3.11)

project (axi_pinlevel_example)

add_executable(axi_pinlevel_example sc_main.cpp)

target_include_directories(axi_pinlevel_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (axi_pinlevel_example PUBLIC tlm-interfaces)
target_link_libraries (axi_pinlevel_example PUBLIC scc)
target_link_libraries (axi_pinlevel_example LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (axi_pinlevel_example LINK_PUBLIC ${CMAKE_DL_LIBS})
