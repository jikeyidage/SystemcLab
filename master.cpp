#include "master.h"

void Master::basic_read_request(sc_uint<32> addr, sc_uint<32> &rd_data) {
    // Address phase: wait for next clock edge
    wait(HCLK.posedge_event());
    
    // Set address phase signals
    HADDR.write(addr);
    HTRANS.write(HTRANS_NONSEQ);
    HWRITE.write(false);  // Read operation
    HBURST.write(HBURST_SINGLE);
    
    // Data phase: wait for HREADY to be HIGH
    // We need to wait at least one clock cycle to enter data phase
    wait(HCLK.posedge_event());
    
    // Wait until HREADY is HIGH (slave is ready)
    while (!HREADY.read()) {
        wait(HCLK.posedge_event());
    }
    
    // Read data is valid when HREADY is HIGH
    rd_data = HRDATA.read();
    
    // Clear transaction signals
    HTRANS.write(HTRANS_IDLE);
}

void Master::basic_write_request(sc_uint<32> addr, sc_uint<32> wr_data) {
    // Address phase: wait for next clock edge
    wait(HCLK.posedge_event());
    
    // Set address phase signals
    HADDR.write(addr);
    HTRANS.write(HTRANS_NONSEQ);
    HWRITE.write(true);   // Write operation
    HBURST.write(HBURST_SINGLE);
    HWDATA.write(wr_data);
    
    // Data phase: wait for HREADY to be HIGH
    // We need to wait at least one clock cycle to enter data phase
    wait(HCLK.posedge_event());
    
    // Keep HWDATA stable until HREADY is HIGH
    while (!HREADY.read()) {
        wait(HCLK.posedge_event());
    }
    
    // Write is complete when HREADY is HIGH
    // Clear transaction signals
    HTRANS.write(HTRANS_IDLE);
}

void Master::pipelined_read_request(sc_uint<32> addr, sc_uint<32> &rd_data) {
    // Address phase: set signals immediately (no wait before setting)
    HADDR.write(addr);
    HTRANS.write(HTRANS_NONSEQ);
    HWRITE.write(false);  // Read operation
    HBURST.write(HBURST_SINGLE);
    
    // Spawn a thread to handle data phase asynchronously
    sc_spawn(
        [this, &rd_data]() {
            // Wait for next clock edge to enter data phase
            wait(HCLK.posedge_event());
            
            // Wait until HREADY is HIGH
            while (!HREADY.read()) {
                wait(HCLK.posedge_event());
            }
            
            // Read data when HREADY is HIGH
            rd_data = HRDATA.read();
            
            // Clear transaction signals
            HTRANS.write(HTRANS_IDLE);
        },
        "pipelined_read_data_phase"
    );
    
    // Return immediately without waiting
}

void Master::pipelined_write_request(sc_uint<32> addr, sc_uint<32> wr_data) {
    // Address phase: set signals immediately
    HADDR.write(addr);
    HTRANS.write(HTRANS_NONSEQ);
    HWRITE.write(true);   // Write operation
    HBURST.write(HBURST_SINGLE);
    HWDATA.write(wr_data);
    
    // Spawn a thread to handle data phase asynchronously
    sc_spawn(
        [this, wr_data]() {
            // Wait for next clock edge to enter data phase
            wait(HCLK.posedge_event());
            
            // Keep HWDATA stable until HREADY is HIGH
            while (!HREADY.read()) {
                wait(HCLK.posedge_event());
            }
            
            // Write is complete, clear transaction signals
            HTRANS.write(HTRANS_IDLE);
        },
        "pipelined_write_data_phase"
    );
    
    // Return immediately without waiting
}

void Master::burst_read_request(sc_uint<32> start_addr, sc_uint<32> burst_len, std::vector<sc_uint<32>> &rd_data) {
    rd_data.clear();
    rd_data.reserve(burst_len);
    
    sc_uint<32> current_addr = start_addr;
    
    // First request: Address phase with NONSEQ
    wait(HCLK.posedge_event());
    HADDR.write(current_addr);
    HTRANS.write(HTRANS_NONSEQ);
    HWRITE.write(false);  // Read operation
    HBURST.write(HBURST_INCR);
    current_addr += 4;
    
    // Wait for data phase of first request
    wait(HCLK.posedge_event());
    
    // First data may have delay, wait for HREADY
    while (!HREADY.read()) {
        wait(HCLK.posedge_event());
    }
    rd_data.push_back(HRDATA.read());
    
    // Subsequent requests: Address phase with SEQ (pipelined)
    for (sc_uint<32> i = 1; i < burst_len; i++) {
        // Set address phase for next request (pipelined with current data phase)
        // This should be set before waiting for next clock edge
        HADDR.write(current_addr);
        HTRANS.write(HTRANS_SEQ);
        HWRITE.write(false);
        
        // Wait for next clock edge (entering data phase for current request)
        wait(HCLK.posedge_event());
        
        // Data phase: subsequent reads have no delay, but check HREADY to be safe
        if (!HREADY.read()) {
            wait(HCLK.posedge_event());
        }
        rd_data.push_back(HRDATA.read());
        current_addr += 4;
    }
    
    // Clear transaction signals
    wait(HCLK.posedge_event());
    HTRANS.write(HTRANS_IDLE);
}

void Master::burst_write_request(sc_uint<32> start_addr, sc_uint<32> burst_len, const std::vector<sc_uint<32>> &wr_data) {
    sc_uint<32> current_addr = start_addr;
    
    // First request: Address phase with NONSEQ
    wait(HCLK.posedge_event());
    HADDR.write(current_addr);
    HTRANS.write(HTRANS_NONSEQ);
    HWRITE.write(true);   // Write operation
    HBURST.write(HBURST_INCR);
    HWDATA.write(wr_data[0]);
    current_addr += 4;
    
    // Wait for data phase of first request
    wait(HCLK.posedge_event());
    
    // First data may have delay, keep HWDATA stable until HREADY
    while (!HREADY.read()) {
        wait(HCLK.posedge_event());
    }
    
    // Subsequent requests: Address phase with SEQ (pipelined)
    for (sc_uint<32> i = 1; i < burst_len; i++) {
        // Set address phase for next request (pipelined with current data phase)
        // This should be set before waiting for next clock edge
        HADDR.write(current_addr);
        HTRANS.write(HTRANS_SEQ);
        HWRITE.write(true);
        HWDATA.write(wr_data[i]);
        
        // Wait for next clock edge (entering data phase for current request)
        wait(HCLK.posedge_event());
        
        // Data phase: subsequent writes have no delay, but check HREADY to be safe
        if (!HREADY.read()) {
            wait(HCLK.posedge_event());
        }
        current_addr += 4;
    }
    
    // Clear transaction signals
    wait(HCLK.posedge_event());
    HTRANS.write(HTRANS_IDLE);
}