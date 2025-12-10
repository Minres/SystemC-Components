project (io_redirector)
if(TARGET Catch2::Catch2WithMain)
	add_executable (${PROJECT_NAME}	test.cpp)
	target_link_libraries (io_redirector LINK_PUBLIC scc-util Catch2::Catch2WithMain)

	#add_test(NAME io_redirector_test COMMAND io_redirector)
	catch_discover_tests(${PROJECT_NAME})
endif()