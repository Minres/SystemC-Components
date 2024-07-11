cmake_minimum_required(VERSION 3.20)
add_executable (transaction_recording scv_tr_recording_example.cpp )
target_link_libraries (transaction_recording LINK_PUBLIC scc)
if(APPLE)
    set_target_properties (transaction_recording PROPERTIES LINK_FLAGS
        -Wl,-U,_sc_main,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif()


add_executable (transaction_recording_ftr scv_tr_recording_example.cpp )
target_compile_definitions(transaction_recording_ftr PUBLIC FTR)
target_link_libraries (transaction_recording_ftr LINK_PUBLIC scc)
if(APPLE)
    set_target_properties (transaction_recording_ftr PROPERTIES LINK_FLAGS
        -Wl,-U,_sc_main,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif()


add_executable (transaction_recording_cftr scv_tr_recording_example.cpp )
target_compile_definitions(transaction_recording_cftr PUBLIC CFTR)
target_link_libraries (transaction_recording_cftr LINK_PUBLIC scc)
if(APPLE)
    set_target_properties (transaction_recording_cftr PROPERTIES LINK_FLAGS
        -Wl,-U,_sc_main,-U,___sanitizer_start_switch_fiber,-U,___sanitizer_finish_switch_fiber)
endif()


add_test(NAME tx_rec_test COMMAND transaction_recording)
add_test(NAME tx_rec_ftr_test COMMAND transaction_recording_ftr)
add_test(NAME tx_rec_cftr_test COMMAND transaction_recording_cftr)