/*
 * sc_main.cpp
 *
 *  Created on: Jul 9, 2014
 *      Author: ejentzsx
 */

#include "scc/report.h"
#include "top.h"
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdlib>
#include <sstream>
#include <systemc.h>
#include <util/logging.h>

#ifdef USE_THREADS
#include <HSSTop.h>

int main(int argc, char* argv[]) {
    /*
    int pid = fork();
    if(pid==0){
        sleep(2);
        exit(hss_main(argc, argv));
    } else {
        return sc_core::sc_elab_and_sim(argc, argv);
    }
    */
    boost::thread_group tg;
    tg.add_thread(new boost::thread(boost::bind(&hss_main, argc, argv)));
    tg.add_thread(new boost::thread(boost::bind(&sc_core::sc_elab_and_sim, argc, argv)));
    tg.join_all();
    return 0;
}
#endif

int sc_main(int argc, char* argv[]) {
    unsigned portNr = 1024;
    bool noClientAccess = false, noSync = false;

    for(int i = 1; i < argc; ++i) {
        if(strncmp(argv[i], "--tlm2Client", 12) == 0 || strncmp(argv[i], "--pythonClient", 14) == 0) {
            noClientAccess = true;
        } else if(strncmp(argv[i], "--noSystemcSync", 15) == 0) {
            noSync = true;
        } else {
            portNr = boost::lexical_cast<unsigned short>(argv[1]);
        }
    }

    top tb("tb");
    LOGGER(DEFAULT)::set_reporting_level(logging::TRACE);
    tb.bridge.thisHostPort.set_value(portNr);
    tb.bridge.isConnectionMaster.set_value(true);
    tb.bridge.writeNoResponse.set_value(false);
    tb.bridge.noSystemcSync.set_value(noSync);

    if(noClientAccess) {
        tb.tg.gotFinalAcc();
        tb.tg.gotIrq();
    }

    try {
        sc_start(SC_ZERO_TIME);
    } catch(std::exception& e) {
        SCCERR() << e.what();
    }

    try {
        sc_start();
    } catch(std::exception& e) {
        SCCERR() << e.what();
    }
    // call sc_Stop if time is exhausted
    if(sc_get_status() != sc_core::SC_STOPPED) {
        sc_stop();
        SCCINFO() << "time out occured @" << sc_time_stamp() << "sec!";
        return EXIT_FAILURE;
    } else {
        SCCINFO() << "FINISHED @" << sc_time_stamp() << "!";
        return EXIT_SUCCESS;
    }
}
