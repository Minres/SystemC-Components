#include <chrono>
#include <stdint.h>
#include <systemc>

using namespace sc_core;

int sc_main(int argc, char* argv[]) {
    const uint64_t NS_VAL = 100000000;

    sc_clock clk("clk", 1, SC_NS);
    sc_time run_time(NS_VAL, SC_NS);

    auto start = std::chrono::high_resolution_clock::now();
    sc_start(run_time);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    double simulated_cycles = (double)NS_VAL;
    double real_us = duration.count();
    double speed_Mhz = simulated_cycles / real_us;
    std::cout << "Simulation speed: " << speed_Mhz << " MHz\n";

    return 0;
}
