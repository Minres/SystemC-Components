cmake_minimum_required(VERSION 3.3)
# Add executable called "transaction_recording" that is built from the source files
# "scv_tr_recording_example.cpp". The extensions are automatically found.
add_executable (ahb_bfm
	sc_main.cpp
)
# Link the executable to the sc_components library. Since the sc_components library has
# public include directories we will use those link directories when building
# transaction_recording
target_link_libraries (ahb_bfm LINK_PUBLIC scc)
