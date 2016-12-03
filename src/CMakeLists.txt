# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
add_library (sc-components scv_tr_sqlite/scv_tr_sqlite.cpp scv_tr_sqlite/sqlite3.c)
# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories (sc-components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../incl)
