cmake_minimum_required(VERSION 3.12)
find_package(fmt)
find_package(Boost COMPONENTS program_options REQUIRED)
add_executable (tcp4tlm_bridge 
    sc_main.cpp
    test_generator.cpp
    initiator.cpp
)
target_link_libraries (tcp4tlm_bridge LINK_PUBLIC scc fmt::fmt)
if(TARGET Boost::program_options)
	target_link_libraries(tcp4tlm_bridge PUBLIC Boost::program_options)
else()
	target_link_libraries(tcp4tlm_bridge PUBLIC ${BOOST_program_options_LIBRARY})
endif()
