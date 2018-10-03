/*
 * time2tick.h
 *
 *  Created on: 03.10.2018
 *      Author: eyck
 */

#ifndef _SCC_TIME2TICK_H_
#define _SCC_TIME2TICK_H_

#include <scc/utilities.h>

class time2tick: public sc_core::sc_module {
    SC_HAS_PROCESS(time2tick);
    sc_core::sc_in<sc_core::sc_time> clk_i;
    sc_core::sc_out<bool> clk_o;

    time2tick(sc_core::sc_module_name nm): sc_core::sc_module(nm){
        SC_THREAD(clocker);
    }
private:
    sc_core::sc_time clk_period;
    void clocker(){
        while(true){
            auto t = clk_i.read();
            if(t==sc_core::SC_ZERO_TIME){
                wait(clk_i.value_changed_event());
                t=clk_i.read();
            }
            clk_o=true;
            wait(t);
            clk_o=false;
            wait(t);
        }
    }
};



#endif /* _SCC_TIME2TICK_H_ */
