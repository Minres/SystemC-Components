add_subdirectory(fst)
add_subdirectory(rapidjson-1.1)
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