cmake_minimum_required(VERSION 3.12)

project(scv-tr VERSION 0.0.1 LANGUAGES CXX)

add_library(${PROJECT_NAME} 
	scv/scv_debug.cpp
	#scv/scv_expression.cpp
	#scv/scv_init_seed.cpp
	scv/scv_introspection.cpp
	#scv/scv_random.cpp
	scv/scv_report.cpp
	scv/scv_tr_text.cpp
	scv/scv_tr.cpp
	scv/scv_util.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/scv)
target_include_directories(${PROJECT_NAME} PUBLIC $ENV{SYSTEMC_HOME}/include)
