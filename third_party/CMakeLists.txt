###############################################################################
# build tlm-interfaces
###############################################################################
add_subdirectory(jsoncpp-1.8)
add_subdirectory(axi_chi)
add_subdirectory(scv-tr)
add_subdirectory(fst)
project(RapidJSON VERSION "1.1.0" LANGUAGES CXX)
set( ${PROJECT_NAME}_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/rapidjson-1.1/include)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/rapidjson-1.1/include)
if(CMAKE_CXX_CPPCHECK)
	target_compile_definitions(${PROJECT_NAME} INTERFACE RAPIDJSON_ENDIAN=RAPIDJSON_LITTLEENDIAN)
endif()