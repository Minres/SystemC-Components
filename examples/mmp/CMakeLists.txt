cmake_minimum_required(VERSION 3.11)

project (mmp_staticmem)

add_executable(mmp_staticmem sc_main.cpp)

target_include_directories(mmp_staticmem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (mmp_staticmem PUBLIC tlm-interfaces)
target_link_libraries (mmp_staticmem PUBLIC scc)
target_link_libraries (mmp_staticmem LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (mmp_staticmem LINK_PUBLIC ${CMAKE_DL_LIBS})
