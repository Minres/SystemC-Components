project (ordered_semaphore)

add_executable(${PROJECT_NAME} 
	test.cpp
	${test_util_SOURCE_DIR}/sc_main.cpp
)
target_link_libraries (${PROJECT_NAME} PUBLIC test_util)

if(NOT THREAD_SANITIZER)
	catch_discover_tests(${PROJECT_NAME})
endif()