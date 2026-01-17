#pragma once
const int CLK_PERIOD = 10;

enum HTRANS_T {
    HTRANS_IDLE = 0,
    HTRANS_BUSY = 1,
    HTRANS_NONSEQ = 2,
    HTRANS_SEQ = 3
};

enum HBURST_T {
    HBURST_SINGLE = 0,
    HBURST_INCR = 1
};

enum HRESP_T {
    HRESP_OKAY = 0,
    HRESP_ERROR = 1
};