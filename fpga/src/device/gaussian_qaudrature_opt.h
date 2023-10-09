#ifndef RYS_QUADRATURE_GAUSSIAN_QUADRATURE_OPT_H
#define RYS_QUADRATURE_GAUSSIAN_QUADRATURE_OPT_H

#include "common/types.h"
#include "common/repeat.h"
#include "common/parameters.h"
#include "streams.h"
#include <hls_stream.h>
#include "internal_types.h"
#include "packed_array.hpp"
#include <algorithm>
#include <cmath>

template<int bp_n, int bp_phi, int ord_rys, int len_d, int len_c>
void load_dcbaox_bufs(
        REPEAT( ARGS_UBA_IN, BP_FIFO_COUNT )
        ba_pt int_cba_buf_x[ord_rys][len_d * len_c],
        ba_pt int_cba_buf_y[ord_rys][len_d * len_c],
        ba_pt int_cba_buf_z[ord_rys][len_d * len_c]
)
{
// for these select quartet classes vitis will generate functionally incorrect hardware unless inlined
#if AM_ABCD == 1010 || AM_ABCD == 1110 || AM_ABCD == 1111 || AM_ABCD == 2010
#pragma hls inline
#else
    AGGRESSIVE_UNROLL_COND_INLINE
#endif

    DEF_UBA_IN_ARR( BP_FIFO_COUNT );

    // this exists separately it improve frequency, counter should always equal: phi_n * bp_n
    int counter = 0;

    for( int phi_n = 0; phi_n < qcf::util::ceil_div( bp_phi, bp_n ); ++phi_n ) // bp_phi is total packets, bp_phi_n is max packets per fifo, rr_unroll_factor is num ubas per packet
    {
        for( int d = 0; d < len_d; ++d )
        {
            for( int c = 0; c < len_c; ++c )
#pragma hls loop_flatten
                    COND_REWIND_PIPELINE_II( 1 )
            {
                uba_pt ubas[bp_n];
#pragma hls array_partition variable=ubas complete dim=0
#pragma hls aggregate variable=ubas

                for( int n = 0; n < bp_n; ++n )
                {
//                    if( phi_n * bp_n + n >= bp_phi ) break;
                    if( counter + n >= bp_phi ) break;
                    ubas[n] = uba_stream_in[n]->read();
                }

                ba_pt bas[bp_n * rr_unroll_factor];
#pragma hls array_partition variable=bas complete dim=0
#pragma hls aggregate variable=bas

                for( int u = 0; u < rr_unroll_factor; ++u )
                    for( int n = 0; n < bp_n; ++n )
                    {
//                        if( phi_n * bp_n + n >= bp_phi ) break;
                        if( counter + n >= bp_phi ) break;
                        for( int b = 0; b < len_b; ++b )
                            for( int a = 0; a < len_a; ++a )
                                bas[n * rr_unroll_factor + u][b][a] = ubas[n][u][b][a];
                    }

                // increment on the last iteration, counter should be: phi_n * bp_n
                if( d == len_d - 1 && c == len_c - 1 ) counter += bp_n;

                for( int u = 0; u < rr_unroll_factor; ++u )
                {
                    int o_base = phi_n * rr_unroll_factor * bp_xyz_unroll_factor + u * bp_xyz_unroll_factor; // using bp_xyz_unroll_factor can be seen as effectively dividing by 3 which converts total ord_rys*nyz to ord_rys

                    for( int u_xyz = 0; u_xyz < bp_xyz_unroll_factor; ++u_xyz )
                    {
                        if( phi_n * bp_n + u_xyz * nxyz >= bp_phi ) break; // we can do this because bp_n aka fifo count is always a multiple of 3, and bp_xyz_unroll_factor = bp_fifo_count / 3
                        if( o_base + u_xyz >= ord_rys ) break; // this catches if phi is perfectly divisible by rr_unroll_factor, and we have a partially filled packet. takes advantage of the fact that bp_n is divisible by 3
                        int bas_base = u * nxyz * bp_xyz_unroll_factor + u_xyz * nxyz;
                        int_cba_buf_x[o_base + u_xyz][d * len_c + c] = bas[bas_base + 0];
                        int_cba_buf_y[o_base + u_xyz][d * len_c + c] = bas[bas_base + 1];
                        int_cba_buf_z[o_base + u_xyz][d * len_c + c] = bas[bas_base + 2];
                    }
                }
            }
        }
    }
}

template<int bp_n, int bp_phi, int ord_rys, int len_d, int len_c>
void load_dcbaox_bufs_pt(
        REPEAT( ARGS_UBA_IN, BP_FIFO_COUNT )
        REPEAT( ARGS_UBA_OUT, BP_FIFO_COUNT )
        ba_pt int_cba_buf_x[ord_rys][len_d * len_c],
        ba_pt int_cba_buf_y[ord_rys][len_d * len_c],
        ba_pt int_cba_buf_z[ord_rys][len_d * len_c]
)
{
// for these select quartet classes vitis will generate functionally incorrect hardware unless inlined
#if AM_ABCD == 1010 || AM_ABCD == 1110 || AM_ABCD == 1111 || AM_ABCD == 2010
#pragma hls inline
#else
AGGRESSIVE_UNROLL_COND_INLINE
#endif

    DEF_UBA_IN_ARR( BP_FIFO_COUNT );
    DEF_UBA_OUT_ARR( BP_FIFO_COUNT );

    // this exists separately it improve frequency, counter should always equal: phi_n * bp_n
    int counter = 0;

    for( int phi_n = 0; phi_n < qcf::util::ceil_div( bp_phi, bp_n ); ++phi_n )
    {
        for( int d = 0; d < len_d; ++d )
        {
            for( int c = 0; c < len_c; ++c )
#pragma hls loop_flatten
                    COND_REWIND_PIPELINE_II( 1 )
            {
                uba_pt ubas[bp_n];
#pragma hls array_partition variable=ubas complete dim=0
#pragma hls aggregate variable=ubas

                for( int n = 0; n < bp_n; ++n )
                {
//                    if( phi_n * bp_n + n >= bp_phi ) break;
                    if( counter + n >= bp_phi ) break;

                    const ap_uint<uba_pt::bit_width>& uba = uba_stream_in[n]->read();
                    ubas[n] = uba;
                    uba_stream_out[n]->write( uba );
                }

                ba_pt bas[bp_n * rr_unroll_factor];
#pragma hls array_partition variable=bas complete dim=0
#pragma hls aggregate variable=bas

                for( int u = 0; u < rr_unroll_factor; ++u )
                    for( int n = 0; n < bp_n; ++n )
                    {
//                        if( phi_n * bp_n + n >= bp_phi ) break;
                        if( counter + n >= bp_phi ) break;

                        for( int b = 0; b < len_b; ++b )
                            for( int a = 0; a < len_a; ++a )
                                bas[n * rr_unroll_factor + u][b][a] = ubas[n][u][b][a];
                    }

                // increment on the last iteration, counter should be: phi_n * bp_n
                if( d == len_d - 1 && c == len_c - 1 ) counter += bp_n;

                for( int u = 0; u < rr_unroll_factor; ++u )
                {
                    int o_base = phi_n * rr_unroll_factor * bp_xyz_unroll_factor + u * bp_xyz_unroll_factor;

                    for( int u_xyz = 0; u_xyz < bp_xyz_unroll_factor; ++u_xyz )
                    {
                        if( phi_n * bp_n + u_xyz * nxyz >= bp_phi ) break; // we can do this because bp_n aka fifo count is always a multiple of 3, and bp_xyz_unroll_factor = bp_fifo_count / 3
                        if( o_base + u_xyz >= ord_rys ) break; // this catches if phi is perfectly divisible by rr_unroll_factor, and we have a partially filled packet. takes advantage of the fact that bp_n is divisible by 3
                        int bas_base = u * nxyz * bp_xyz_unroll_factor + u_xyz * nxyz;
                        int_cba_buf_x[o_base + u_xyz][d * len_c + c] = bas[bas_base + 0];
                        int_cba_buf_y[o_base + u_xyz][d * len_c + c] = bas[bas_base + 1];
                        int_cba_buf_z[o_base + u_xyz][d * len_c + c] = bas[bas_base + 2];
                    }
                }

            }
        }
    }
}

template<int ord_rys, int len_d, int len_c, int ncg_d, int ncg_c, int ncg_b, int ncg_a, int ncg_b_from, int ncg_b_to>
void gaussian_quadrature_split(
        ba_pt int_cba_buf_x[ord_rys][len_d * len_c],
        ba_pt int_cba_buf_y[ord_rys][len_d * len_c],
        ba_pt int_cba_buf_z[ord_rys][len_d * len_c],
        hls::stream<ap_uint<packed_array<fp_t, ncg_b_to - ncg_b_from, ncg_a>::bit_width / 2>>& partial_eris_stream_out_0,
        hls::stream<ap_uint<packed_array<fp_t, ncg_b_to - ncg_b_from, ncg_a>::bit_width / 2>>& partial_eris_stream_out_1
)
{
// for these select quartet classes vitis will generate functionally incorrect hardware unless inlined
#if AM_ABCD == 1010 || AM_ABCD == 1110 || AM_ABCD == 1111 || AM_ABCD == 2010
#pragma hls inline
#else
    AGGRESSIVE_UNROLL_COND_INLINE
#endif

    constexpr qcf::util::gq_seg_index_array_t<ncg_d * ncg_c * 3> indices_outer = qcf::util::collect_gq_segment_accessed_indices<ncg_d, ncg_c>( am_d, am_c );
    constexpr qcf::util::gq_seg_index_array_t<ncg_b * ncg_a * 3> indices_inner = qcf::util::collect_gq_segment_accessed_indices<ncg_b, ncg_a>( am_b, am_a );

    GQ_MAIN_OUTER:
    for( int i = 0; i < ncg_d * ncg_c; i++ )
#pragma hls loop_flatten
            COND_REWIND_PIPELINE_II( 1 )
    {
        packed_array<fp_t, ncg_b_to - ncg_b_from, ncg_a> partial_sum_packet;
        partial_sum_packet.set_zero();

        for( int o = 0; o < ord_rys; ++o )
        {
            ba_pt packet_x = int_cba_buf_x[o][indices_outer.arr[i * 3 + 0].b * len_c + indices_outer.arr[i * 3 + 0].a];
            ba_pt packet_y = int_cba_buf_y[o][indices_outer.arr[i * 3 + 1].b * len_c + indices_outer.arr[i * 3 + 1].a];
            ba_pt packet_z = int_cba_buf_z[o][indices_outer.arr[i * 3 + 2].b * len_c + indices_outer.arr[i * 3 + 2].a];

            for( int igb = ncg_b_from; igb < ncg_b_to; igb++ )
            {
                for( int iga = 0; iga < ncg_a; iga++ )
                {
                    fp_t x = packet_x[indices_inner.arr[(igb * ncg_a + iga) * 3 + 0].b][indices_inner.arr[(igb * ncg_a + iga) * 3 + 0].a]  // [nb|md]μx
                             * packet_y[indices_inner.arr[(igb * ncg_a + iga) * 3 + 1].b][indices_inner.arr[(igb * ncg_a + iga) * 3 + 1].a]  // [nb|md]μy
                             * packet_z[indices_inner.arr[(igb * ncg_a + iga) * 3 + 2].b][indices_inner.arr[(igb * ncg_a + iga) * 3 + 2].a]; // [nb|md]μz
#pragma hls bind_op variable=x op=fmul impl=fulldsp
                    partial_sum_packet[igb - ncg_b_from][iga] = partial_sum_packet[igb - ncg_b_from][iga] + x;
                }
            }

        }
        constexpr int bit_width = packed_array<fp_t, ncg_b_to - ncg_b_from, ncg_a>::bit_width;
        ap_uint<bit_width> result;
        result = partial_sum_packet;

        partial_eris_stream_out_0.write( result.range( bit_width / 2 - 1, 0 ) );
        partial_eris_stream_out_1.write( result.range( bit_width - 1, bit_width / 2 ) );
    }
}


#endif
