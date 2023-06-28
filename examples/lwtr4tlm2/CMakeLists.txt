project (lwtr4tlm2)

add_executable(lwtr4tlm2 sc_main.cpp)
target_link_libraries (lwtr4tlm2 PUBLIC scc)
target_link_libraries (lwtr4tlm2 LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (lwtr4tlm2 LINK_PUBLIC ${CMAKE_DL_LIBS})
