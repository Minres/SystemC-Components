cmake_minimum_required(VERSION 3.11)

project (scc-tlm_target_bfs-example)

add_executable(${PROJECT_NAME} sc_main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (${PROJECT_NAME} PUBLIC tlm-interfaces)
target_link_libraries (${PROJECT_NAME} PUBLIC scc)
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS})
