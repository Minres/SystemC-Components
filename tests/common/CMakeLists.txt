project (test_util)

add_library(${PROJECT_NAME} factory.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries (${PROJECT_NAME} PUBLIC scc Catch2::Catch2)
