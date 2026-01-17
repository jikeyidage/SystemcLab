#include "testbench.h"
#include <unordered_set>

const int BASIC_TESTS = 20;

void Testbench::test_basic_read_write() {
    // test read
    wait(HCLK.posedge_event());
    printf("[T=%d ns] [Testbench] Testing basic read write\n", (int)sc_time_stamp().to_double() / 1000);
    printf("[T=%d ns] [Testbench] Basic Test 1: Read correctness\n", (int)sc_time_stamp().to_double() / 1000);
    for (int i = 0; i < BASIC_TESTS; i++) {
        printf("\n[T=%d ns] [Testbench] Basic Test 1: Round %d start\n", (int)sc_time_stamp().to_double() / 1000, i + 1);
        sc_uint<32> rd_data = 0;
        sc_uint<32> rd_addr = rand() % 0x2000;
        sc_uint<32> gt_rd_data = (slave->memory)[rd_addr];
        printf("[T=%d ns] [Testbench] Basic Test 1: Reading addr=0x%x\n", (int)sc_time_stamp().to_double() / 1000, rd_addr.to_uint());
        
        // blocking read request
        sc_time before = sc_time_stamp();
        master->basic_read_request(rd_addr, rd_data);
        sc_time after = sc_time_stamp();
        // check timing
        if(after - before > sc_time((READ_DELAY_CYCLES + 2) * CLK_PERIOD, SC_NS)) {
            fprintf(stderr, "[T=%d ns] [Testbench] Basic Test 1 Round %d Failed: Basic read took more time than expected\n\n", 
                (int)sc_time_stamp().to_double() / 1000, i + 1);
            sc_stop();
        } 
        // check results
        if(gt_rd_data != rd_data) {
            fprintf(stderr, "[T=%d ns] [Testbench] Basic Test 1 Round %d Failed: %x != %x\n\n", 
                (int)sc_time_stamp().to_double() / 1000, i + 1, gt_rd_data.to_uint(), rd_data.to_uint());
            sc_stop();
        } else {
            printf("[T=%d ns] [Testbench] Basic Test 1: Round %d passed\n\n", (int)sc_time_stamp().to_double() / 1000, i + 1);
        }
    }
    // test write
    wait(HCLK.posedge_event());
    printf("[T=%d ns] [Testbench] Basic Test 2: Write correctness\n", (int)sc_time_stamp().to_double() / 1000);
    for (int i = 0; i < BASIC_TESTS; i++) {
        printf("\n[T=%d ns] [Testbench] Basic Test 2: Round %d start\n", (int)sc_time_stamp().to_double() / 1000, i + 1);
        sc_uint<32> wr_data = rand() % 0x4000;
        sc_uint<32> wr_addr = rand() % 0x2000;
        
        printf("[T=%d ns] [Testbench] Basic Test 2: Writing addr=0x%x, data=0x%x\n", (int)sc_time_stamp().to_double() / 1000, wr_addr.to_uint(), wr_data.to_uint());
        // blocking write request
        sc_time before = sc_time_stamp();
        master->basic_write_request(wr_addr, wr_data);
        sc_time after = sc_time_stamp();
        // check timing
        if(after - before > sc_time((WRITE_DELAY_CYCLES + 2) * CLK_PERIOD, SC_NS)) {
            fprintf(stderr, "[T=%d ns] [Testbench] Basic Test 2 Round %d Failed: Basic write took more time than expected\n\n", 
                (int)sc_time_stamp().to_double() / 1000, i + 1);
            sc_stop();
        }
        // check results
        if(slave->memory[wr_addr] != wr_data) {
            std::cerr << "[T=" << sc_time_stamp() << "] [Testbench] Basic Test 2: Write Failed: " << slave->memory[wr_addr] << "!=" << wr_data << std::endl;
            sc_stop();
        } else {
            printf("[T=%d ns] [Testbench] Basic Test 2: Round %d passed\n\n", (int)sc_time_stamp().to_double() / 1000, i + 1);
        }
    }
    // test passed
    wait(HCLK.posedge_event());
    printf("[T=%d ns] [Testbench] Basic read and write test passed\n\n", (int)sc_time_stamp().to_double() / 1000);
    score += 60;
    basic_finished.notify();
}

void Testbench::spawned_read_request(int i, int test_id, sc_uint<32> rd_addr) {
    sc_uint<32> gt_rd_data = (slave->memory)[rd_addr];
    sc_uint<32> rd_data = 0;
    sc_time before = sc_time_stamp();
    master->pipelined_read_request(rd_addr, rd_data);
    sc_time after = sc_time_stamp();
    if(after != before) {
        fprintf(stderr, "[T=%d ns] [Testbench] Pipelined Test %d Round %d Failed: Pipeline read should not block\n\n", 
            (int)sc_time_stamp().to_double() / 1000, test_id, i + 1);
        sc_stop();
    }
    for (int i = 0; i < READ_DELAY_CYCLES + 2; i++)
        wait(HCLK.posedge_event());
    wait(1, SC_NS);
    if(gt_rd_data != rd_data) {
        fprintf(stderr, "[T=%d ns] [Testbench] Pipelined Test %d Round %d Failed: %x != %x\n\n", 
            (int)sc_time_stamp().to_double() / 1000, test_id, i + 1, gt_rd_data.to_uint(), rd_data.to_uint());
        sc_stop();
    } else {
        printf("[T=%d ns] [Testbench] Pipelined Test %d: Round %d passed\n\n", 
            (int)sc_time_stamp().to_double() / 1000, test_id, i + 1);
    }
}

void Testbench::spawned_write_request(int i, int test_id, sc_uint<32> wr_data, sc_uint<32> wr_addr) {
    sc_time before = sc_time_stamp();
    master->pipelined_write_request(wr_addr, wr_data);
    sc_time after = sc_time_stamp();
    if(after != before) {
        fprintf(stderr, "[T=%d ns] [Testbench] Pipelined Test %d Round %d Failed: Pipeline write should not block\n\n", 
            (int)sc_time_stamp().to_double() / 1000, test_id, i + 1);
        sc_stop();
    }
    for (int i = 0; i < WRITE_DELAY_CYCLES + 2; i++)
        wait(HCLK.posedge_event());
    wait(1, SC_NS);
    sc_uint<32> gt_data = (slave->memory)[wr_addr];
    if(gt_data != wr_data) {
        fprintf(stderr, "[T=%d ns] [Testbench] Pipelined Test %d Round %d Failed: %x != %x\n\n", 
            (int)sc_time_stamp().to_double() / 1000, test_id, i + 1, gt_data.to_uint(), wr_data.to_uint());
        sc_stop();
    } else {
        printf("[T=%d ns] [Testbench] Pipelined Test %d: Round %d passed\n\n", 
            (int)sc_time_stamp().to_double() / 1000, test_id, i + 1);
    }
}


void Testbench::test_pipelined_read_write() {
    wait(basic_finished);
    // test read
    wait(HCLK.posedge_event());
    printf("\n");
    printf("[T=%d ns] [Testbench] Testing pipelined read write\n", (int)sc_time_stamp().to_double() / 1000);
    printf("[T=%d ns] [Testbench] Pipelined Test 1: Pipelined read correctness\n", (int)sc_time_stamp().to_double() / 1000);
    for (int i = 0; i < BASIC_TESTS; i++) {
        printf("\n[T=%d ns] [Testbench] Pipelined Test 1: Round %d start\n", (int)sc_time_stamp().to_double() / 1000, i + 1);
        sc_uint<32> rd_addr = rand() % 0x2000;
        printf("[T=%d ns] [Testbench] Pipelined Test 1: Reading addr=0x%x\n", (int)sc_time_stamp().to_double() / 1000, rd_addr.to_uint());
        sc_spawn(
            [&, i, rd_addr]() {
                this->spawned_read_request(i, 1, rd_addr);
            },
            ("test_read" + std::to_string(i)).c_str()
        );
        for (int i = 0; i < READ_DELAY_CYCLES + 1; i++)
            wait(HCLK.posedge_event());
    }
    // test write
    for (int i = 0; i < READ_DELAY_CYCLES + 1; i++)
        wait(HCLK.posedge_event());
    
    printf("[T=%d ns] [Testbench] Pipelined Test 2: Pipelined write correctness\n", (int)sc_time_stamp().to_double() / 1000);
    // gen distinct random write addresses
    std::unordered_set<int> write_addresses;
    while (write_addresses.size() < BASIC_TESTS) {
        write_addresses.insert(rand() % 0x2000);
    }
    for (int i = 0; i < BASIC_TESTS; i++) {
        printf("\n[T=%d ns] [Testbench] Pipelined Test 2: Round %d start\n", (int)sc_time_stamp().to_double() / 1000, i + 1);
        sc_uint<32> wr_data = rand() % 0x4000;
        sc_uint<32> wr_addr = *(std::next(write_addresses.begin(), i));
        
        printf("[T=%d ns] [Testbench] Pipelined Test 2: Writing addr=0x%x, data=0x%x\n", (int)sc_time_stamp().to_double() / 1000, wr_addr.to_uint(), wr_data.to_uint());
        sc_spawn(
            [&, i, wr_data, wr_addr]() {
                this->spawned_write_request(i, 2, wr_data, wr_addr);
            },
            ("test_write_" + std::to_string(i)).c_str()
        );
        for (int i = 0; i < WRITE_DELAY_CYCLES + 1; i++)
            wait(HCLK.posedge_event());
    }
    // test passed
    for (int i = 0; i < WRITE_DELAY_CYCLES + 1; i++)
        wait(HCLK.posedge_event());
    printf("[T=%d ns] [Testbench] Pipelined read and write test passed\n\n", (int)sc_time_stamp().to_double() / 1000);
    score += 20;
    pipelined_finished.notify();
}

void Testbench::test_burst_read_write() {
    wait(pipelined_finished);
    wait(HCLK.posedge_event());
    // test burst read
    printf("[T=%d ns] [Testbench] Testing burst read write\n", (int)sc_time_stamp().to_double() / 1000);
    printf("[T=%d ns] [Testbench] Burst Test 1: Burst read correctness\n", (int)sc_time_stamp().to_double() / 1000);
    for (int i = 0; i < BASIC_TESTS; i++) {
        printf("\n[T=%d ns] [Testbench] Burst Test 1: Round %d start\n", (int)sc_time_stamp().to_double() / 1000, i + 1);
        sc_uint<32> rd_addr = rand() % 0x2000;
        int burst_length = 4 * (rand() % 4 + 1); // 4, 8, 12, 16
        printf("[T=%d ns] [Testbench] Burst Test 1: Reading start addr=0x%x, burst length=%d\n", 
            (int)sc_time_stamp().to_double() / 1000, rd_addr.to_uint(), burst_length);
        std::vector<sc_uint<32>> rd_data, gt_rd_data;
        for (int j = 0; j < burst_length; j++) {
            gt_rd_data.push_back(slave->memory[rd_addr + j]);
        }
        // blocking burst read request
        sc_time before = sc_time_stamp();   
        master->burst_read_request(rd_addr, burst_length, rd_data);
        sc_time after = sc_time_stamp();   
        // check timing
        if(after - before > sc_time((burst_length + READ_DELAY_CYCLES + 1) * CLK_PERIOD, SC_NS)) {
            fprintf(stderr, "[T=%d ns] [Testbench] Burst Test 1 Round %d Failed: Burst read took more time than expected\n\n", 
                (int)sc_time_stamp().to_double() / 1000, i + 1);
            sc_stop();
        } 
        // check results
        if (rd_data.size() != burst_length) {
            fprintf(stderr, "[T=%d ns] [Testbench] Burst Test 1 Round %d Failed: Read data size incorrect\n\n", 
                (int)sc_time_stamp().to_double() / 1000, i + 1);
            sc_stop();
        } else {
            for (int j = 0; j < burst_length; j++) {
                if (rd_data[j] != gt_rd_data[j]) {
                    fprintf(stderr, "[T=%d ns] [Testbench] Burst Test 1 Round %d Failed at burst index %d: %x != %x\n\n", 
                        (int)sc_time_stamp().to_double() / 1000, i + 1, j, rd_data[j].to_uint(), gt_rd_data[j].to_uint());
                    sc_stop();
                }
            }
        }
        wait(HCLK.posedge_event());
    }

    // test burst write
    wait(HCLK.posedge_event());
    printf("[T=%d ns] [Testbench] Burst Test 2: Burst write correctness\n", (int)sc_time_stamp().to_double() / 1000);
    for (int i = 0; i < BASIC_TESTS; i++) {
        printf("\n[T=%d ns] [Testbench] Burst Test 2: Round %d start\n", (int)sc_time_stamp().to_double() / 1000, i + 1);
        sc_uint<32> wr_addr = rand() % 0x2000;
        int burst_length = 4 * (rand() % 4 + 1); // 4, 8, 12, 16
        std::vector<sc_uint<32>> wr_data, gt_data;
        for (int j = 0; j < burst_length; j++) {
            wr_data.push_back(rand() % 0x4000);
        }
        printf("[T=%d ns] [Testbench] Burst Test 2: Writing start addr=0x%x, burst length=%d\n", 
            (int)sc_time_stamp().to_double() / 1000, wr_addr.to_uint(), burst_length);
        
        // blocking burst write request
        sc_time before = sc_time_stamp();   
        master->burst_write_request(wr_addr, burst_length, wr_data);
        sc_time after = sc_time_stamp();   
        // check timing
        if(after - before > sc_time((burst_length + WRITE_DELAY_CYCLES + 1) * CLK_PERIOD, SC_NS)) {
            fprintf(stderr, "[T=%d ns] [Testbench] Burst Test 2 Round %d Failed: Burst write took more time than expected\n\n", 
                (int)sc_time_stamp().to_double() / 1000, i + 1);
            sc_stop();
        } 
        // check results
        for (int j = 0; j < burst_length; j++) {
            sc_uint<32> gt_data = slave->memory[wr_addr + j];
            if (wr_data[j] != gt_data) {
                fprintf(stderr, "[T=%d ns] [Testbench] Burst Test 2 Round %d Failed at burst index %d: %x != %x\n\n", 
                    (int)sc_time_stamp().to_double() / 1000, i + 1, j, wr_data[j].to_uint(), gt_data.to_uint());
                sc_stop();
            }
        }
        wait(HCLK.posedge_event());
    }
    wait(HCLK.posedge_event());
    printf("[T=%d ns] [Testbench] Burst read and write test passed\n\n", (int)sc_time_stamp().to_double() / 1000);
    score += 20;
    sc_stop();
}