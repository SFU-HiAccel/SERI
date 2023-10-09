#ifndef RYS_QUADRATURE_RECURRENCE_RELATIONS_H
#define RYS_QUADRATURE_RECURRENCE_RELATIONS_H

#include "common/types.h"
#include "internal_types.h"
#include "common/repeat.h"
#include "common/parameters.h"
#include "streams.h"
#include "mdim_range.hpp"

using namespace qcf;

/**
 * compute vertical & horizontal recurrence relations
 */
template<
        int_t am_b,
        int_t am_d,
        int_t am_n,
        int_t am_m,
        int_t len_a,
        int_t len_b,
        int_t len_c,
        int_t len_d,
        int_t len_n,
        int_t len_m,
        int_t ord_rys
>
void recurrence_relations(
        qcf::istream<D_WIDTH( xyz_derived_pt::bit_width ) >& xyz_derived_stream_in,
        qcf::istream<D_WIDTH( rys_wt_pt::bit_width ) >& rys_wt_stream_in,
        qcf::istream<D_WIDTH( tb_b_pt::bit_width ) >& tb_b_stream_in,
        qcf::istream<D_WIDTH( tb_c_pt::bit_width ) >& tb_c_stream_in,
#if BP_BYPASS == 0
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 2 ) >& ucba_stream_out_0,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 2 ) >& ucba_stream_out_1,
#elif BP_BYPASS == 1
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_0,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_1,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_2,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_3,
#endif
        n_t n
)
{
#pragma hls inline
    constexpr int eri_tc = ord_rys * nxyz;

//    constexpr int_t ixyz_vals[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2};
//    constexpr int_t iORD_vals[] = {0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6};

    N_LOOP:
    for( int i_n = 0; i_n < n; ++i_n )
    AGGRESSIVE_UNROLL_COND_PIPELINE
    {
        xyz_derived_pt xyz_derived;
        rys_wt_pt wt_rys;
        tb_b_pt tb_cci;
        tb_c_pt tb_ccd;
#pragma hls aggregate variable=xyz_derived
#pragma hls aggregate variable=wt_rys
#pragma hls aggregate variable=tb_cci
#pragma hls aggregate variable=tb_ccd

        // vitis seems to be smart enough adjust down the bitwidth of this, so the div and mod shouldn't be that expensive
        ORD_RYS_XYZ_LOOP:
        for( int outer = 0; outer < qcf::util::ceil_div( ord_rys * nxyz, rr_unroll_factor ); outer++ ) // loop goes to bp_phi
#pragma hls loop_flatten
#pragma hls pipeline II=len_d
        {
            ap_uint<cba_pt::bit_width> packets[rr_unroll_factor][len_d];
#pragma hls array_partition variable=packets complete dim=0

            for( int unrolled = 0; unrolled < rr_unroll_factor; unrolled++ )
#pragma hls unroll
            {
                int net_idx = outer * rr_unroll_factor + unrolled;
                if( net_idx >= ord_rys * nxyz ) break;
                int_t iORD = net_idx / nxyz;
                int_t ixyz = net_idx % nxyz;

                if( outer == 0 && unrolled == 0 )
                {
                    xyz_derived = xyz_derived_stream_in.read();
                    wt_rys = rys_wt_stream_in.read();
                    tb_cci = tb_b_stream_in.read();
                    tb_ccd = tb_c_stream_in.read();
                }

                fp_t int4d_dmbn[len_d][len_m][len_b][len_n];
#pragma hls array_partition variable=int4d_dmbn complete dim=0
//#pragma hls bind_storage variable=packets type=ram_s2p impl=bram

                int4d_dmbn[0][0][0][0] = wt_rys[iORD];
                AM_N_LOOP:
                for( int_t idxn { 0 }; idxn < am_n; ++idxn )
#pragma hls unroll
                {
                    fp_t t = idxn ? idxn * tb_cci[2][iORD] * int4d_dmbn[0][0][0][idxn - 1] : 0.0;
                    int4d_dmbn[0][0][0][idxn + 1] = t + tb_ccd[1][iORD][ixyz] *
                                                        int4d_dmbn[0][0][0][idxn];
                }
                int_t iORD_1 = net_idx / nxyz; // replicating this decreases fanout and enforces less grouping resulting in less congestion
                int_t ixyz_1 = net_idx % nxyz; // replicating this decreases fanout and enforces less grouping resulting in less congestion
                AM_M_LOOP:
                for( int_t idxm { 0 }; idxm < am_m; ++idxm )
#pragma hls unroll
                {
                    fp_t t = idxm ? idxm * tb_cci[1][iORD_1] * int4d_dmbn[0][idxm - 1][0][0] : 0.0;
                    int4d_dmbn[0][idxm + 1][0][0] = t + tb_ccd[0][iORD_1][ixyz_1] *
                                                        int4d_dmbn[0][idxm][0][0];
                    AM_M_LOOP_N:
                    for( int_t idxn { 0 }; idxn < am_n; ++idxn )
                    {
                        fp_t t = idxn ? idxn * tb_cci[2][iORD_1] * int4d_dmbn[0][idxm + 1][0][idxn - 1] : 0.0;
                        int4d_dmbn[0][idxm + 1][0][idxn + 1] = t
                                                               + (idxm + 1) * tb_cci[0][iORD_1] * int4d_dmbn[0][idxm][0][idxn]
                                                               + tb_ccd[1][iORD_1][ixyz_1] * int4d_dmbn[0][idxm + 1][0][idxn];
                    }
                }
                int_t ixyz_3 = net_idx % nxyz; // replicating this decreases fanout and enforces less grouping resulting in less congestion
                VRR_LOOP:
                for( int_t idxm { 0 }; idxm < len_m; ++idxm )
#pragma hls unroll
                {
                    VRR_LOOP_B:
                    for( int_t idxb { 0 }; idxb < am_b; ++idxb )
                    {
                        VRR_LOOP_N:
                        for( int_t idxn { 0 }; idxn < am_n - idxb; ++idxn )
                        {
                            int4d_dmbn[0][idxm][idxb + 1][idxn] =
                                    int4d_dmbn[0][idxm][idxb][idxn + 1] +
                                    int4d_dmbn[0][idxm][idxb][idxn] * xyz_derived[0][ixyz_3];
                        }
                    }
                }
                int_t ixyz_4 = net_idx % nxyz; // replicating this decreases fanout and enforces less grouping resulting in less congestion
                HRR_LOOP:
                for( int_t idxd { 0 }; idxd < am_d; ++idxd )
#pragma hls unroll
                {
                    HRR_LOOP_M:
                    for( int_t idxm { 0 }; idxm < am_m - idxd; ++idxm )
                    {
                        HRR_LOOP_B:
                        for( int_t idxb { 0 }; idxb < len_b; ++idxb )
                        {
                            HRR_LOOP_A:
                            for( int_t idxa { 0 }; idxa < len_a; ++idxa )
                            {
                                int4d_dmbn[idxd + 1][idxm][idxb][idxa] =
                                        int4d_dmbn[idxd][idxm + 1][idxb][idxa] +
                                        int4d_dmbn[idxd][idxm][idxb][idxa] * xyz_derived[1][ixyz_4];
                            }
                        }
                    }
                }
                STORE_LOOP_OUTER:
                for( int_t idxd { 0 }; idxd < len_d; ++idxd )
#pragma hls pipeline II=1
                {
                    cba_pt packed_dcbaox;
                    STORE_LOOP_INNER:
                    for( int_t idxc { 0 }; idxc < len_c; ++idxc )
#pragma hls unroll
                    {
                        for( int_t idxb { 0 }; idxb < len_b; ++idxb )
                        {
                            for( int_t idxa { 0 }; idxa < len_a; ++idxa )
                            {
                                packed_dcbaox[idxc][idxb][idxa] = int4d_dmbn[idxd][idxc][idxb][idxa];
                            }
                        }
                    }
                    packets[unrolled][idxd] = packed_dcbaox;
                }
            }

            MERGE_WRITE_LOOP:
            for( int_t idxd { 0 }; idxd < len_d; ++idxd )
            {
                // merge unrolled into final value
                ap_uint<ucba_pt::bit_width> packet;
                for( int u = 0; u < rr_unroll_factor; ++u )
                    packet.range( (int) (cba_pt::bit_width * (u + 1)) - 1, (int) (cba_pt::bit_width * u) ) = (ap_uint<cba_pt::bit_width>) packets[u][idxd];
#if BP_BYPASS == 0
                ucba_stream_out_0.write( packet.range( ucba_pt::bit_width / 2 - 1, 0 ) );
                ucba_stream_out_1.write( packet.range( ucba_pt::bit_width - 1, ucba_pt::bit_width / 2 ) );
#elif BP_BYPASS == 1
                constexpr int quarter = ucba_pt::bit_width / 4;
                ucba_stream_out_0.write( packet.range( quarter - 1, 0 ) );
                ucba_stream_out_1.write( packet.range( quarter * 2 - 1, quarter ) );
                ucba_stream_out_2.write( packet.range( quarter * 3 - 1, quarter * 2 ) );
                ucba_stream_out_3.write( packet.range( quarter * 4 - 1, quarter * 3 ) );
#endif
            }
        }

    }
}

#endif