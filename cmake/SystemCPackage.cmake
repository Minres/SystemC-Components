# a small wrapper to map SLS, OSCI SystemC (from environment) and conan package to the same variables
if(NOT SystemC_FOUND)
    if(USE_CWR_SYSTEMC)
        find_package(SLSSystemC REQUIRED)
        set(SystemC_LIBRARIES SystemC::systemc)
    elseif(USE_VCS_SYSTEMC)
	find_package(VCSSystemC REQUIRED)
        set(SystemC_LIBRARIES SystemC::systemc)
    elseif(USE_NCSC_SYSTEMC)
        find_package(XMSystemC REQUIRED)
        set(SystemC_LIBRARIES SystemC::systemc)
    elseif(USE_MTI_SYSTEMC)
        find_package(MTISystemC REQUIRED)
        set(SystemC_LIBRARIES SystemC::systemc)
    else()
    	set(CMAKE_FIND_USE_PACKAGE_REGISTRY FALSE)
    	set(CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY TRUE)
        find_package(SystemCLanguage QUIET)
        if(TARGET SystemC::systemc) # conan find_package_generator or the cmake of an SystemC installation
            set(SystemC_FOUND true)
            set(SystemC_LIBRARIES SystemC::systemc)
            find_package(systemc-scv QUIET)
            if(systemc-scv_FOUND)
                set(SCV_FOUND TRUE)
                set(SCV_LIBRARIES systemc-scv::systemc-scv)
            endif()
            find_package(systemc-cci QUIET)
            if(systemc-cci_FOUND)
                set(CCI_FOUND TRUE)
                set(CCI_LIBRARIES systemc-cci::systemc-cci)
            endif()
        elseif(TARGET CONAN_PKG::systemc) # conan cmake targets
            set(SystemC_FOUND true)
            set(SystemC_LIBRARIES CONAN_PKG::systemc)
            if(TARGET CONAN_PKG::systemc-scv)
                set(SCV_FOUND TRUE)
                set(SCV_LIBRARIES CONAN_PKG::systemc-scv)
            endif()
            if(TARGET CONAN_PKG::systemc-cci)
                set(CCI_FOUND TRUE)
                set(CCI_LIBRARIES CONAN_PKG::systemc-cci)
            endif()
        else()
            find_package(OSCISystemC)
            set(SystemC_LIBRARIES SystemC::systemc)
        endif()
    endif()
###############################################################################
#
###############################################################################
    file(WRITE "${CMAKE_BINARY_DIR}/test_mylib.cpp" "
#include <systemc.h>
int
main( int argc, char* argv[] ) {
    printf(\"%d\\n\", SC_VERSION_MAJOR);
    printf(\"%d\\n\", SC_VERSION_MINOR);
    printf(\"%d\\n\", SC_VERSION_PATCH);
    return 0;
}
int sc_main(int argc, char* argv[]) {
    printf(\"%d\", SC_VERSION_MAJOR); return 0;
}
")
    set(CMAKE_TRY_COMPILE_CONFIGURATION  ${CMAKE_BUILD_TYPE})
    try_run(RUN_RESULT COMPILE_RESULT
        "${CMAKE_BINARY_DIR}/test_run"
        "${CMAKE_BINARY_DIR}/test_mylib.cpp"
        LINK_LIBRARIES SystemC::systemc
        COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
        RUN_OUTPUT_VARIABLE RUN_OUTPUT
    )
    if(COMPILE_RESULT AND RUN_RESULT EQUAL 0)
        # Normalize line endings (handle CRLF and LF)
        string(REPLACE "\r\n" "\n" RUN_OUTPUT "${RUN_OUTPUT}")
        # Split into list elements
        string(REPLACE "\n" ";" OUTPUT_LINES "${RUN_OUTPUT}")
        list(GET OUTPUT_LINES 0 SC_VERSION_MAJOR)
        list(GET OUTPUT_LINES 1 SC_VERSION_MINOR)
        list(GET OUTPUT_LINES 2 SC_VERSION_PATCH)
        message(STATUS "Detected SystemC version ${SC_VERSION_MAJOR}.${SC_VERSION_MINOR}.${SC_VERSION_PATCH}")
    else()
        #message(STATUS "COMPILE_OUTPUT:\n${COMPILE_OUTPUT}")
        #message(STATUS "RUN_OUTPUT:\n${RUN_OUTPUT}")
        message(FATAL_ERROR "❌ SystemC does not compile correctly, pls check your setup or report at https://github.com/Minres/SystemC-Components/issues.")
    endif()
###############################################################################
#
###############################################################################
endif()
