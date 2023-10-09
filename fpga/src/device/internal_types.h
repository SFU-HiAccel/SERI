#ifndef QC_FPGA_INTERNAL_TYPES_H
#define QC_FPGA_INTERNAL_TYPES_H

// needs to be defined before inclusion of ap_int.h
#define AP_INT_MAX_W 8192

#include "common/types.h"
#include "packed_array.hpp"
#include "common/qc_util_advanced.h"
#include "ap_int.h"

#define D_WIDTH( d_width ) ap_uint< d_width >

#if AGGRESSIVE_UNROLL == 1
#define AGGRESSIVE_UNROLL_COND_INLINE PRAGMA( hls inline )
#define AGGRESSIVE_UNROLL_COND_PIPELINE PRAGMA( hls pipeline II=gq_std_tc style=frp )
#define AGGRESSIVE_UNROLL_COND_DATAFLOW_PIPELINE PRAGMA( hls pipeline II=gq_std_tc style=frp )
#else
#define AGGRESSIVE_UNROLL_COND_INLINE PRAGMA( hls inline off )
#define AGGRESSIVE_UNROLL_COND_PIPELINE
#define AGGRESSIVE_UNROLL_COND_DATAFLOW_PIPELINE PRAGMA( hls dataflow )
#endif

#if REWIND_PIPELINES == 1 // only pipelines in a dataflow region need to be rewind
#if PREFER_FRP == 1
#define COND_REWIND_PIPELINE_II( ii ) PRAGMA( hls pipeline II=ii rewind style=frp )
#else
#define COND_REWIND_PIPELINE_II( ii ) PRAGMA( hls pipeline II=ii rewind )
#endif
#else
#if PREFER_FRP == 1
#define COND_REWIND_PIPELINE_II( ii ) PRAGMA( hls pipeline II=ii style=frp )
#else
#define COND_REWIND_PIPELINE_II( ii ) PRAGMA( hls pipeline II=ii )
#endif
#endif

typedef ap_uint<21> n_t;

// k_preparation -> k_recurrence_relations
typedef packed_array<fp_t, 5, nxyz> xyz_derived_pt;
typedef packed_array<fp_t, ord_rys> rys_wt_pt;
typedef packed_array<fp_t, 3, ord_rys> tb_b_pt;
typedef packed_array<fp_t, 2, ord_rys, nxyz> tb_c_pt;

// k_recurrence_relations -> k_buffer_permutation
typedef packed_array<fp_t, len_c, len_b, len_a> cba_pt;

//  k_permutation
typedef packed_array<fp_t, rr_unroll_factor, len_c, len_b, len_a> ucba_pt; // P = rr_unroll * len_b * len_a

// k_buffer_permutation -> k_gaussian_quadrature
typedef packed_array<fp_t, rr_unroll_factor, len_b, len_a> uba_pt;

// k_gaussian_quadrature
typedef packed_array<fp_t, len_b, len_a> ba_pt;
constexpr int ncg_b_gq_a = ncg_b / 3;
constexpr int ncg_b_gq_b = ncg_b - ncg_b_gq_a;
typedef packed_array<fp_t, ncg_b_gq_a, ncg_a> ba_cart_eris_a_pt;
typedef packed_array<fp_t, ncg_b_gq_b, ncg_a> ba_cart_eris_b_pt;

// k_recurrence_relations -> k_compress_store
typedef packed_array<fp_t, ncg_b, ncg_a> ba_cart_eris_pt;

#define ARGS_UBA_IN( z ) qcf::istream<ap_uint<uba_pt::bit_width>>& uba_stream_in_##z,
#define PASS_UBA_IN( z ) uba_stream_in_##z,
#define PASS_UBA_IN_PTRS( z ) &uba_stream_in_##z,
#define DEF_UBA_IN_ARR( z ) qcf::istream<ap_uint<uba_pt::bit_width>>* uba_stream_in[] = {REPEAT( PASS_UBA_IN_PTRS, z)}

#define ARGS_UBA_OUT( z ) qcf::ostream<ap_uint<uba_pt::bit_width>>& uba_stream_out_##z,
#define PASS_UBA_OUT( z ) uba_stream_out_##z,
#define PASS_UBA_OUT_PTRS( z ) &uba_stream_out_##z,
#define DEF_UBA_OUT_ARR( z ) qcf::ostream<ap_uint<uba_pt::bit_width>>* uba_stream_out[] = {REPEAT( PASS_UBA_OUT_PTRS, z)}


#endif //QC_FPGA_INTERNAL_TYPES_H
