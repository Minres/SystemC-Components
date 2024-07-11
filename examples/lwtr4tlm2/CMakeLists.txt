project (lwtr4tlm2)

add_executable(${PROJECT_NAME} sc_main.cpp)
target_link_libraries (${PROJECT_NAME} PUBLIC scc)
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC ${CMAKE_DL_LIBS})
if(APPLE)
    set_target_properties (${PROJECT_NAME} PROPERTIES LINK_FLAGS
        -Wl,-U,_sc_main,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif()

