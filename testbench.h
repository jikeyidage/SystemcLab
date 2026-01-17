#include "systemc.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include "constants.h"
#include "master.h"  
#include "slave.h"   


SC_MODULE(Testbench) {
    sc_clock HCLK;
    
    Master* master;
    Slave* slave;

    // signals
    sc_signal<sc_uint<32>, SC_MANY_WRITERS> HADDR;
    sc_signal<sc_uint<32>, SC_MANY_WRITERS> HWDATA;
    sc_signal<sc_uint<32>, SC_MANY_WRITERS> HRDATA;
    sc_signal<bool, SC_MANY_WRITERS> HREADY;
    sc_signal<sc_uint<2>, SC_MANY_WRITERS> HTRANS;
    sc_signal<bool, SC_MANY_WRITERS> HWRITE;
    sc_signal<sc_uint<1>, SC_MANY_WRITERS> HRESP;
    sc_signal<sc_uint<1>, SC_MANY_WRITERS> HBURST;

    SC_CTOR(Testbench) 
        : HCLK("HCLK", 10, SC_NS, 0.5, 5, SC_NS, false) 
    {
        srand(time(NULL));
        master = new Master("master");
        slave = new Slave("slave");
        
        // master connection
        master->HCLK(HCLK);
        master->HADDR(HADDR);
        master->HWDATA(HWDATA);
        master->HRDATA(HRDATA);
        master->HREADY(HREADY);
        master->HTRANS(HTRANS);
        master->HWRITE(HWRITE);
        master->HRESP(HRESP);
        master->HBURST(HBURST);
        
        // slave connection
        slave->HCLK(HCLK);
        slave->HADDR(HADDR);
        slave->HWDATA(HWDATA);
        slave->HTRANS(HTRANS);
        slave->HWRITE(HWRITE);
        slave->HBURST(HBURST);
        slave->HRDATA(HRDATA);
        slave->HREADY(HREADY);
        slave->HRESP(HRESP);

        SC_THREAD(test_basic_read_write);
        SC_THREAD(test_pipelined_read_write);
        SC_THREAD(test_burst_read_write);
    }

    ~Testbench() {
        delete master;
        delete slave;
    }

    sc_event basic_finished, pipelined_finished;
    int score = 0;

    void test_basic_read_write();
    void test_pipelined_read_write();
    void spawned_read_request(int i, int test_id, sc_uint<32> rd_addr);
    void spawned_write_request(int i, int test_id, sc_uint<32> wr_data, sc_uint<32> wr_addr);
    void test_burst_read_write();
};