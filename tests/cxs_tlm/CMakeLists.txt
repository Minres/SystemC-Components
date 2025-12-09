project (cxs_tlm)
add_executable(${PROJECT_NAME} 
	csx_packet_test.cpp
	${test_util_SOURCE_DIR}/sc_main.cpp
)
target_link_libraries (${PROJECT_NAME} PUBLIC scc::busses test_util)

if(NOT THREAD_SANITIZER)
	catch_discover_tests(${PROJECT_NAME})
endif()