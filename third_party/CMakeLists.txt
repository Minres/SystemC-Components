add_subdirectory(fst)
project(RapidJSON VERSION "1.1.0" LANGUAGES CXX)
set( ${PROJECT_NAME}_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/rapidjson-1.1/include)
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/rapidjson-1.1/include)
if(CMAKE_CXX_CPPCHECK)
	target_compile_definitions(${PROJECT_NAME} INTERFACE RAPIDJSON_ENDIAN=RAPIDJSON_LITTLEENDIAN)
endif()
###############################################################################
# build tlm-interfaces
###############################################################################
if(SystemC_FOUND)
	add_subdirectory(axi_chi)
	add_subdirectory(scv-tr)
    if(NOT TARGET systemc-cci::systemc-cci AND NOT TARGET SystemC::cci)
        add_subdirectory(cci-1.0.0)
    endif()
	add_subdirectory(lwtr4sc/src)
endif()