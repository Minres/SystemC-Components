cmake_minimum_required(VERSION 3.12)
add_executable (transaction_recording
	scv_tr_recording_example.cpp
)
target_link_libraries (transaction_recording LINK_PUBLIC scc)

add_test(NAME tx_rec_test COMMAND transaction_recording)