#include "slave.h"
#include "constants.h"
#include <iostream>
#include <iomanip> // For std::hex, std::dec

enum SLAVE_STATE {
    READY = 0, 
    WAIT = 1,
    PROCESS = 2,
};


void Slave::receiveMasterRequest() {
    if (HTRANS.read() == HTRANS_NONSEQ || HTRANS.read() == HTRANS_SEQ) {
        registered_addr = HADDR.read();
        registered_write = HWRITE.read();
        registered_htrans = HTRANS.read();
        
        // set read/write latency when NONSEQ
        if (registered_write) {
            delay_counter = HTRANS.read() == HTRANS_NONSEQ ? WRITE_DELAY_CYCLES : 0;
            std::cout << "[T=" << sc_time_stamp() << "] Received write request from master" << std::endl;
        } else {
            delay_counter = HTRANS.read() == HTRANS_NONSEQ ? READ_DELAY_CYCLES : 0;
            std::cout << "[T=" << sc_time_stamp() << "] Received read request from master" << std::endl;
        }

        if (delay_counter == 0) {
            processMasterRequest();
        } else {
            --delay_counter;
            if (delay_counter >= 1) {
                state.write(SLAVE_STATE::WAIT);
            } else if (delay_counter == 0) {
                state.write(SLAVE_STATE::PROCESS);
            }
            HREADY.write(false);
        }
    }
}

void Slave::processMasterRequest() {
    assert(delay_counter == 0);
    if (registered_write) {
        std::cout << "[T=" << sc_time_stamp() << "] Slave is processing master write request" << std::endl;
        if (registered_addr < 0x40000) {
            write_addr = registered_addr;
            next_cycle_write = true;
            
            std::cout << "[T=" << sc_time_stamp() << "] Slave prepare to write at memory addr=0x" 
                      << std::hex << registered_addr.to_uint() << std::dec << std::endl;
            HRESP.write(HRESP_OKAY);
        } else { // out of bound
            std::cout << "[T=" << sc_time_stamp() << "] Slave write addr 0x" 
                      << std::hex << registered_addr.to_uint() << std::dec << " is out of bound!" << std::endl;
            HRESP.write(HRESP_ERROR);
        }
    } else {
        std::cout << "[T=" << sc_time_stamp() << "] Slave is processing master read request" << std::endl;
        if (registered_addr < 0x40000) {
            HRDATA.write(memory[registered_addr]); 
            std::cout << "[T=" << sc_time_stamp() << "] Slave read 0x" 
                      << std::hex << memory[registered_addr].to_uint() << std::dec 
                      << " at memory addr=0x" << std::hex << registered_addr.to_uint() << std::dec << std::endl;
            HRESP.write(HRESP_OKAY);
        } else { // out of bound
            std::cout << "[T=" << sc_time_stamp() << "] Slave read addr 0x" 
                      << std::hex << registered_addr.to_uint() << std::dec << " is out of bound!" << std::endl;
            HRDATA.write(0xFFFFFFFF);
            HRESP.write(HRESP_ERROR);
        }
    }
}

void Slave::logic() {
    // process pending write
    if (next_cycle_write) {
        memory[write_addr] = HWDATA.read();
        std::cout << "[T=" << sc_time_stamp() << "] Slave write 0x" 
                  << std::hex << HWDATA.read().to_uint() << std::dec 
                  << " at memory addr=0x" << std::hex << write_addr.to_uint() << std::dec << std::endl;
        next_cycle_write = false;
    }
    // main logic
    if (state.read() == SLAVE_STATE::READY) {
        receiveMasterRequest();
    } else if (state.read() == SLAVE_STATE::WAIT) {
        std::cout << "[T=" << sc_time_stamp() << "] Slave is waiting" << std::endl;
        if((--delay_counter) == 0) {
            state.write(SLAVE_STATE::PROCESS);
        }
    } else { // process
        HREADY.write(true);
        processMasterRequest();
        state.write(SLAVE_STATE::READY);
    }
}