project (ace_ace_example)

add_executable(ace_ace_example sc_main.cpp)
target_include_directories(ace_ace_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (ace_ace_example PUBLIC tlm-interfaces)
target_link_libraries (ace_ace_example PUBLIC scc)
target_link_libraries (ace_ace_example LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (ace_ace_example LINK_PUBLIC ${CMAKE_DL_LIBS})
