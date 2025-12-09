project (ace_pin_level)

add_executable(${PROJECT_NAME} 
	ace_narrow_burst_test.cpp
	${test_util_SOURCE_DIR}/sc_main.cpp
)
target_link_libraries (${PROJECT_NAME} PUBLIC test_util)
if(NOT THREAD_SANITIZER)
	catch_discover_tests(${PROJECT_NAME})
endif()