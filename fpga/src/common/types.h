#ifndef QC_FPGA_TYPES_H
#define QC_FPGA_TYPES_H

#include "constants.h"
#include "parameters.h"

using namespace qcf;

using fp_t = float;
using int_t = unsigned int;

struct alignas(32)  qp_t
{
    fp_t xyz_abcd[four][nxyz];
    fp_t exp_abcd[four];
};

struct alignas(32) rys_t
{
    fp_t rt[ord_rys];
    fp_t wt[ord_rys];
};

struct alignas(32) packed_eri_t
{
    short data[pack_count];
};

#define ARGS_CART_ERIS( z ) packed_eri_t* cart_eris_##z,
#define PASS_CART_ERIS( z ) cart_eris_##z,

#endif //QC_FPGA_TYPES_H
