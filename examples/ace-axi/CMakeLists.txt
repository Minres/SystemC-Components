cmake_minimum_required(VERSION 3.11)

project (ace_axi_example)

find_package(SystemCPackage)

get_target_property(LIBS SystemC::systemc INTERFACE_LINK_LIBRARIES)
message("====> Libs are ${LIBS}")
add_executable(ace_axi_example sc_main.cpp)
target_include_directories(ace_axi_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (ace_axi_example PUBLIC tlm-interfaces)
target_link_libraries (ace_axi_example PUBLIC scc-sysc)
target_link_libraries (ace_axi_example LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (ace_axi_example LINK_PUBLIC ${CMAKE_DL_LIBS})
