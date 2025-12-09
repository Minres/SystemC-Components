cmake_minimum_required(VERSION 3.12)
find_package(fmt)
find_package(Boost COMPONENTS program_options REQUIRED)
add_executable (sim_performance 
    sc_main.cpp
	pkt_sender.cpp
	pkt_switch.cpp
	top.cpp
)
target_link_libraries (sim_performance LINK_PUBLIC scc)
target_link_libraries (sim_performance LINK_PUBLIC scc fmt::fmt)
if(TARGET Boost::program_options)
	target_link_libraries(sim_performance PUBLIC Boost::program_options)
else()
	target_link_libraries(sim_performance PUBLIC ${BOOST_program_options_LIBRARY})
endif()

#Mateo Done erst count 16384
foreach(x RANGE 1 10)
   add_test(NAME sim_performance_16x16_${x} COMMAND sim_performance)
   add_test(NAME sim_performance_32x32_${x} COMMAND sim_performance --dim 32 --count 50000)
   add_test(NAME sim_performance_64x64_${x} COMMAND sim_performance --dim 64 --count 100000)
endforeach()
