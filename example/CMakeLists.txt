cmake_minimum_required (VERSION 2.8.12)
# Add executable called "recordingExample" that is built from the source files
# "scv_tr_recording_example.cpp". The extensions are automatically found.
add_executable (recordingExample scv_tr_recording_example.cpp)
# Link the executable to the Hello library. Since the Hello library has
# public include directories we will use those link directories when building
# recordingExample
target_link_libraries (recordingExample LINK_PUBLIC sc-components)
target_link_libraries (recordingExample LINK_PUBLIC ${SystemC_LIBRARIES})
target_link_libraries (recordingExample LINK_PUBLIC ${SCV_LIBRARIES})
target_link_libraries (recordingExample LINK_PUBLIC ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (recordingExample LINK_PUBLIC ${CMAKE_DL_LIBS})
