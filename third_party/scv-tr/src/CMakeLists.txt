cmake_minimum_required(VERSION 3.12)

project(scv-tr VERSION 0.0.1 LANGUAGES CXX)

add_library(${PROJECT_NAME} 
	scv-tr/scv_introspection.cpp
	scv-tr/scv_report.cpp
	scv-tr/scv_tr_text.cpp
	scv-tr/scv_tr.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if(TARGET SystemC::systemc)
    target_link_libraries(${PROJECT_NAME} SystemC::systemc)
elseif(TARGET CONAN_PKG::systemc)
    target_link_libraries(${PROJECT_NAME} CONAN_PKG::systemc)
else()
	target_include_directories(${PROJECT_NAME} PUBLIC $ENV{SYSTEMC_HOME}/include)
endif()