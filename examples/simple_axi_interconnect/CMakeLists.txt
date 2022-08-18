project (simple_axi_interconnect)

add_executable(simple_axi_interconnect sc_main.cpp)
target_include_directories(simple_axi_interconnect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (simple_axi_interconnect PUBLIC tlm-interfaces)
target_link_libraries (simple_axi_interconnect PUBLIC scc)
target_link_libraries (simple_axi_interconnect LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (simple_axi_interconnect LINK_PUBLIC ${CMAKE_DL_LIBS})
