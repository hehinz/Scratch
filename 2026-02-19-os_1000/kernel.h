#pragma once

/*
 * long is either 32 or 64 bit depending on architecture
 */

struct SbiRet {
    long error;
    union {
        long s_value;
        unsigned long u_value;
    }
};
