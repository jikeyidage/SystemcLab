#pragma once

#include <systemc.h>
#include <iostream>
#include "constants.h"

SC_MODULE(Master) {
    // neccessary ports, do not change
    sc_in<bool> HCLK;          
    sc_in<sc_uint<32>> HRDATA;
    sc_in<bool> HREADY;
    sc_in<sc_uint<1>> HRESP;

    sc_out<sc_uint<32>> HADDR; 
    sc_out<sc_uint<32>> HWDATA;
    sc_out<sc_uint<2>> HTRANS; 
    sc_out<bool> HWRITE;
    sc_out<sc_uint<1>> HBURST;

    SC_CTOR(Master) {}
    
public:
    void basic_read_request(sc_uint<32> addr, sc_uint<32> &rd_data);

    void basic_write_request(sc_uint<32> addr, sc_uint<32> wr_data);

    void pipelined_read_request(sc_uint<32> addr, sc_uint<32> &rd_data);

    void pipelined_write_request(sc_uint<32> addr, sc_uint<32> wr_data);

    void burst_read_request(sc_uint<32> start_addr, sc_uint<32> burst_len, std::vector<sc_uint<32>> &rd_data);

    void burst_write_request(sc_uint<32> start_addr, sc_uint<32> burst_len, const std::vector<sc_uint<32>> &wr_data);
};

