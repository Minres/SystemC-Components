project (scp_example)

add_executable(scp_example sc_main.cpp)
target_include_directories(scp_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (scp_example PUBLIC scc)
target_link_libraries (scp_example LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (scp_example LINK_PUBLIC ${CMAKE_DL_LIBS})
