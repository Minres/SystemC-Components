project (memory_subsys)
add_executable(${PROJECT_NAME} 
	memory_test.cpp
	${test_util_SOURCE_DIR}/sc_main.cpp
)
target_link_libraries (${PROJECT_NAME} PUBLIC scc::components test_util)

if(NOT THREAD_SANITIZER)
	catch_discover_tests(${PROJECT_NAME})
endif()