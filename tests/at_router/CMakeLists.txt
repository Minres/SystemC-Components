project(at_router)
add_executable(${PROJECT_NAME}
    at_router_test.cpp
    ${test_util_SOURCE_DIR}/sc_main.cpp
)
target_link_libraries(${PROJECT_NAME} PUBLIC scc::components test_util)

if(NOT THREAD_SANITIZER)
    catch_discover_tests(${PROJECT_NAME})
endif()
