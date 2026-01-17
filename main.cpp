#include <systemc.h>
#include "testbench.h"
#include "constants.h"

const int MAX_CYCLES = 10000;

int sc_main(int argc, char* argv[]) {
    Testbench top("TESTBENCH");
    sc_start(MAX_CYCLES * CLK_PERIOD, SC_NS);
    printf("SystemC AHB Simulation Finished. Score: %d/100\n", top.score);
    return 0;
}