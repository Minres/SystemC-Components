find_package(Doxygen)
if(NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is needed to build the documentation.")
endif()
set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

set(DOXYGEN_PROJECT_BRIEF "SystemC components library")
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE  README.md)
# exclude json code
# doxygen settings can be set here, prefixed with "DOXYGEN_"
set(DOXYGEN_SOURCE_BROWSER YES)
set(DOXYGEN_EXTRACT_ALL NO)
set(DOXYGEN_EXTRACT_PRIVATE NO)
set(DOXYGEN_SEARCHENGINE NO)
set(DOXYGEN_GENERATE_HTML YES)
set(DOXYGEN_GENERATE_MAN NO)
set(DOXYGEN_GENERATE_LATEX NO)
set(DOXYGEN_GENERATE_XML YES)
set(DOXYGEN_OUTPUT_DIRECTORY "${scc_BINARY_DIR}/scc")
set(DOXYGEN_GENERATE_TREEVIEW  YES)
set(DOXYGEN_HTML_EXTRA_STYLESHEET ${CMAKE_CURRENT_SOURCE_DIR}/doxygen-awesome.css)
#set(DOXYGEN_HTML_EXTRA_STYLESHEET ${CMAKE_CURRENT_SOURCE_DIR}/doxygen-awesome.css ${CMAKE_CURRENT_SOURCE_DIR}/doxygen-awesome-sidebar-only.css)
# this target will only be built if specifically asked to.
# run "make scc-docs" to create the doxygen documentation
doxygen_add_docs(
    scc-docs
    ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/third_party/axi_chi/axi  ${PROJECT_SOURCE_DIR}/third_party/axi_chi/chi  ${PROJECT_SOURCE_DIR}/README.md
    ALL COMMENT "Generate API-documents for SCC."
)
install(DIRECTORY ${scc_BINARY_DIR}/scc DESTINATION ${CMAKE_INSTALL_DOCDIR}/doxy)

###############################################################################
# build read the docs 
###############################################################################
find_package(Sphinx QUIET)
if(Sphinx_FOUND)
	set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
	set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/docs/sphinx)
	
	add_custom_target(Sphinx ALL
	                  COMMAND ${SPHINX_EXECUTABLE} -b html
	                  # Tell Breathe where to find the Doxygen output
	                  -Dbreathe_projects.SCC=${scc_BINARY_DIR}/scc/xml
	                  ${SPHINX_SOURCE} ${SPHINX_BUILD}
	                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	                  COMMENT "Generating documentation with Sphinx"
	                  DEPENDS scc-docs)
	                  
	# Add an install target to install the docs
	include(GNUInstallDirs)
	install(DIRECTORY ${SPHINX_BUILD} DESTINATION ${CMAKE_INSTALL_DOCDIR}/rtd)
endif()