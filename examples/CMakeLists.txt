if(NOT USE_CWR_SYSTEMC)
	add_subdirectory(ace-axi)
	add_subdirectory(ace-ace)
	add_subdirectory(axi-axi)
    add_subdirectory(axi4_tlm-pin-tlm)
    add_subdirectory(axi4lite_tlm-pin-tlm)
	add_subdirectory(ahb_bfm)
	add_subdirectory(simple_system)
	add_subdirectory(transaction_recording)
    add_subdirectory(scc-tlm_target_bfs)
    add_subdirectory(lwtr)
    add_subdirectory(lwtr4tlm2)
    add_subdirectory(lwtr4axi)
    add_subdirectory(scp)
endif()

