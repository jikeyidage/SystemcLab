#pragma once

#include <systemc.h>
#include <iostream>
#include "constants.h"

const int READ_DELAY_CYCLES = 3; 
const int WRITE_DELAY_CYCLES = 2; 

SC_MODULE(Slave) {
    // slave ports
    sc_in<bool> HCLK;           
    sc_in<sc_uint<32>> HADDR;
    sc_in<sc_uint<32>> HWDATA;
    sc_in<sc_uint<2>> HTRANS;
    sc_in<bool> HWRITE;
    sc_in<sc_uint<1>> HBURST;

    sc_out<sc_uint<32>> HRDATA;
    sc_out<bool> HREADY;   
    sc_out<sc_uint<1>> HRESP;
    
    // slave memory: valid address from 0-0x40000
    sc_uint<32> memory[0x40000];

    sc_signal<sc_uint<2>> state;
    
    sc_uint<32> registered_addr;
    bool registered_write;
    sc_uint<2> registered_htrans;
    int delay_counter; 

    bool next_cycle_write = false;
    sc_uint<32> write_addr;

    SC_CTOR(Slave) {
        SC_METHOD(logic);
        sensitive << HCLK.pos();
        
        for (int i = 0; i < 0x40000; i++) {
            memory[i] = 0xDEAD0000 + i; 
        }
        HREADY.initialize(true);
        HRESP.initialize(HRESP_OKAY); 
        HRDATA.initialize(0);
        delay_counter = 0;
    }

    void receiveMasterRequest();
    void processMasterRequest();
    void logic();
};