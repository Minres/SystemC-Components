project (ahb_bfm)

add_executable(${PROJECT_NAME} sc_main.cpp)
target_link_libraries (${PROJECT_NAME} PUBLIC scc)
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS})
