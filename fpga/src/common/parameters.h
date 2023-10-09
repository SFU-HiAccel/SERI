#ifndef QC_FPGA_PARAMETERS_H
#define QC_FPGA_PARAMETERS_H

#include "qc_util.h"

#ifndef AM_ABCD
#define AM_ABCD 3131
#endif

// include after AM_ABCD is set
#include "param_config.h"

// angular momentum values
constexpr int am_a = qcf::util::extract_nth_digit( AM_ABCD, 3 );
constexpr int am_b = qcf::util::extract_nth_digit( AM_ABCD, 2 );
constexpr int am_c = qcf::util::extract_nth_digit( AM_ABCD, 1 );
constexpr int am_d = qcf::util::extract_nth_digit( AM_ABCD, 0 );
constexpr int am_abcd[4] { am_a, am_b, am_c, am_d };

// eri result compression n-bits
constexpr int eri_bits = 16;
constexpr int eri_port_bit_width = 256;
constexpr int pack_count = eri_port_bit_width / eri_bits;

// derived constants
constexpr int ncg_a = qcf::util::calc_ncg( am_a ); // number of Cart-GTOs on a
constexpr int ncg_b = qcf::util::calc_ncg( am_b ); // number of Cart-GTOs on b
constexpr int ncg_c = qcf::util::calc_ncg( am_c ); // number of Cart-GTOs on c
constexpr int ncg_d = qcf::util::calc_ncg( am_d ); // number of Cart-GTOs on d
constexpr int num_cart_eris_quartet = qcf::util::calc_num_eris_per_quartet( am_abcd ); // number of Cartesian ERIs per quartet
constexpr int ord_rys = qcf::util::calc_ord_rys( am_abcd ); // order of the Rys polynomials

/* - for a, b, c, d --------------------------------------------------------------------- */
constexpr int len_a = am_a + 1;                  /* length for the a-dimension        */
constexpr int len_b = am_b + 1;                  /* length for the b-dimension        */
constexpr int len_c = am_c + 1;                  /* length for the c-dimension        */
constexpr int len_d = am_d + 1;                  /* length for the d-dimension        */
/* - for [ab| → [n0| and |cd] → |m0] --------------------------------------------------- */
constexpr int am_n = am_a + am_b;               /* total angular momentum for [ab|   */
constexpr int am_m = am_c + am_d;               /* total angular momentum for |cd]   */
constexpr int len_n = am_n + 1;                 /* length for the ab-dimension       */
constexpr int len_m = am_m + 1;                 /* length for the cd-dimension       */

// tc standard values (without additional unrolling)
constexpr int rr_std_tc = ord_rys * 3 * len_d;
constexpr int gq_std_tc = ncg_d * ncg_c;

// rr & bp scaling params
constexpr int rr_unroll_factor = AM_ABCD == 3310 || AM_ABCD == 3311 ? 3 : qcf::util::ceil_div( rr_std_tc, gq_std_tc );
constexpr int bp_d = len_d;
constexpr int bp_delta = len_c;
constexpr int bp_phi = qcf::util::ceil_div( (ord_rys * 3), rr_unroll_factor );

constexpr int bp_n_gq_ideal = qcf::util::ceil_div( bp_d * bp_delta * (ord_rys * 3), gq_std_tc * (rr_unroll_factor) ); // bp_phi is broken down in this formula to prevent rounding from causing erronious values
constexpr int bp_n_ideal = bp_n_gq_ideal;

constexpr int bp_fifo_count = (REWIND_PIPELINES) ? qcf::util::ceil_div( bp_n_ideal, 3 ) * 3 : 3; // MUST BE A MULTIPLE OF 3
constexpr int bp_xyz_unroll_factor = bp_fifo_count / 3;

// number of ports for store
constexpr int num_eris_ports = qcf::util::ceil_div( ncg_b * ncg_a, pack_count );

// validate macro values match with expected values
static_assert( ORD_RYS == ord_rys, "ORD_RYS macro and calculated ord_rys do not match" );
static_assert( NUM_ERIS_PORTS == num_eris_ports, "NUM_ERIS_PORTS macro and calculated num_eris_ports do not match" );
static_assert( BP_FIFO_COUNT == bp_fifo_count, "BP_FIFO_COUNT macro and calculated bp_fifo_count do not match" );
static_assert( bp_fifo_count % 3 == 0, "bp_fifo_count must be divisible by 3 (nxyz)" );
static_assert( GQ_KERNEL_SPLIT == (am_b > 0 ? 2 : 1), "GQ_KERNEL_SPLIT macro and calculated expected value do not match" );


#endif //QC_FPGA_PARAMETERS_H
