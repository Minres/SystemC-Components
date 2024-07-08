add_subdirectory(fst)
add_subdirectory(rapidjson-1.1)
###############################################################################
# build tlm-interfaces
###############################################################################
if(SystemC_FOUND)
	option(USE_CCI_100 "" OFF)
	add_subdirectory(axi_chi)
	add_subdirectory(scv-tr)
    if(NOT TARGET systemc-cci::systemc-cci AND NOT TARGET SystemC::cci)
    	if(USE_CCI_100)
        	add_subdirectory(cci-1.0.0)
        else()
         	add_subdirectory(cci-1.0.1)
        endif()
    endif()
	add_subdirectory(lwtr4sc/src)
endif()