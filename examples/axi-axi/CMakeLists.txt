project (axi_axi_example)

add_executable(${PROJECT_NAME} sc_main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (${PROJECT_NAME} PUBLIC scc)
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS})
if(APPLE)
    set_target_properties (${PROJECT_NAME} PROPERTIES LINK_FLAGS
        -Wl,-U,_sc_main,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif()
