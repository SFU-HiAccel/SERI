#include "internal_types.h"
#include "recurrence_relations.h"
#include "hls_stream.h"

using namespace qcf;


extern "C" {
void k_rr_b(
        hls::stream<D_WIDTH( xyz_derived_pt::bit_width ) >& xyz_derived_stream_in,
        hls::stream<D_WIDTH( rys_wt_pt::bit_width ) >& rys_wt_stream_in,
        hls::stream<D_WIDTH( tb_b_pt::bit_width ) >& tb_b_stream_in,
        hls::stream<D_WIDTH( tb_c_pt::bit_width ) >& tb_c_stream_in,

        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 2 ) >& ucba_stream_out_1,

        hls::stream<n_t>& n_stream_in,
        hls::stream<n_t>& n_stream_out
)
{
#pragma hls interface ap_ctrl_none port=return

    n_t n = n_stream_in.read();
    n_stream_out.write( n );


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

            int unrolled = 1;
//            for( int unrolled = 0; unrolled < rr_unroll_factor; unrolled++ )
            {
                int net_idx = outer * rr_unroll_factor + unrolled;
                if( net_idx >= ord_rys * nxyz ) break;
                int_t iORD = net_idx / nxyz;
                int_t ixyz = net_idx % nxyz;

                if( outer == 0 )
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
                    ucba_stream_out_1.write( packed_dcbaox );
                }
            }

        }

    }
}
}