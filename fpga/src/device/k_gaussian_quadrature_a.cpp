#include "internal_types.h"
#include "common/qc_util_advanced.h"
#include <hls_stream.h>
#include "gaussian_qaudrature_opt.h"
#include "split_merge_stream.h"

using namespace qcf;
using qcf::util::partial_array_t;
using qcf::util::gq_seg_index_t;


#if AM_ABCD == 3333 || AM_ABCD == 3332 // could technically be enabled for all rr_unroll_factor == 1 and bp_fifo_count = 3
template<> // specialization for AM_ABCD = 3333 to optimize and allow placement without PCN
void load_dcbaox_bufs_pt<3, bp_phi, ord_rys, len_d, len_c>(
        REPEAT( ARGS_UBA_IN, BP_FIFO_COUNT )
        REPEAT( ARGS_UBA_OUT, BP_FIFO_COUNT )
        ba_pt int_cba_buf_x[ord_rys][len_d * len_c],
        ba_pt int_cba_buf_y[ord_rys][len_d * len_c],
        ba_pt int_cba_buf_z[ord_rys][len_d * len_c]
)
{
    AGGRESSIVE_UNROLL_COND_INLINE
    constexpr int bp_n = bp_fifo_count;

    for( int phi_n = 0; phi_n < qcf::util::ceil_div( bp_phi, bp_n ); ++phi_n ) // bp_phi is total packets, bp_phi_n is max packets per fifo, rr_unroll_factor is num ubas per packet
    {
        for( int d = 0; d < len_d; ++d )
        {
            for( int c = 0; c < len_c; ++c )
#pragma hls loop_flatten
                    COND_REWIND_PIPELINE_II( 1 )
            {
                int o = phi_n; // simplification for rr_unroll_factor = 1, and fifo_count = 3

//               because rr_unroll_factor = 1, ba_pt and uba_pt have same width
                ap_uint<ba_pt::bit_width> uba_x = uba_stream_in_0.read();
                ap_uint<ba_pt::bit_width> uba_y = uba_stream_in_1.read();
                ap_uint<ba_pt::bit_width> uba_z = uba_stream_in_2.read();

                uba_stream_out_0.write( uba_x );
                uba_stream_out_1.write( uba_y );
                uba_stream_out_2.write( uba_z );

                int_cba_buf_x[o][d * len_c + c] = (ba_pt) uba_x;
                int_cba_buf_y[o][d * len_c + c] = (ba_pt) uba_y;
                int_cba_buf_z[o][d * len_c + c] = (ba_pt) uba_z;

            }
        }
    }
}
#endif


extern "C" {
void k_gaussian_quadrature_a(
#if BP_BYPASS == 0
        REPEAT( ARGS_UBA_IN, BP_FIFO_COUNT )
        REPEAT( ARGS_UBA_OUT, BP_FIFO_COUNT )
#elif BP_BYPASS == 1
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_in_0,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_in_1,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_in_2,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_in_3,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_0,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_1,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_2,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_3,
#endif
        hls::stream<ap_uint<ba_cart_eris_a_pt::bit_width / 2>>& partial_eris_stream_out_0,
        hls::stream<ap_uint<ba_cart_eris_a_pt::bit_width / 2>>& partial_eris_stream_out_1,
        hls::stream<n_t>& n_stream_in,
        hls::stream<n_t>& n_stream_out
)
{
#pragma hls interface ap_ctrl_none port=return

    constexpr int rys_unroll = ord_rys;

    n_t n = n_stream_in.read();
    n_stream_out.write( n );

    for( int i_n = 0; i_n < n; ++i_n )
    AGGRESSIVE_UNROLL_COND_DATAFLOW_PIPELINE
    {
        ba_pt int_cba_buf_x[rys_unroll][len_d * len_c];
        ba_pt int_cba_buf_y[rys_unroll][len_d * len_c];
        ba_pt int_cba_buf_z[rys_unroll][len_d * len_c];
#if BP_BYPASS == 0
#pragma hls array_partition variable=int_cba_buf_x complete dim=1
#pragma hls array_partition variable=int_cba_buf_y complete dim=1
#pragma hls array_partition variable=int_cba_buf_z complete dim=1
#pragma hls bind_storage variable=int_cba_buf_x type=ram_2p impl=bram
#pragma hls bind_storage variable=int_cba_buf_y type=ram_2p impl=bram
#pragma hls bind_storage variable=int_cba_buf_z type=ram_2p impl=bram
#elif BP_BYPASS == 1
#pragma hls array_partition variable=int_cba_buf_x complete dim=0
#pragma hls array_partition variable=int_cba_buf_y complete dim=0
#pragma hls array_partition variable=int_cba_buf_z complete dim=0
#endif
#pragma hls aggregate variable=int_cba_buf_x
#pragma hls aggregate variable=int_cba_buf_y
#pragma hls aggregate variable=int_cba_buf_z
#if AGGRESSIVE_UNROLL == 0
#pragma hls stream variable=int_cba_buf_x type=pipo depth=2
#pragma hls stream variable=int_cba_buf_y type=pipo depth=2
#pragma hls stream variable=int_cba_buf_z type=pipo depth=2
#endif

#if BP_BYPASS == 0
        load_dcbaox_bufs_pt<bp_fifo_count, bp_phi, rys_unroll, len_d, len_c>( REPEAT( PASS_UBA_IN, BP_FIFO_COUNT ) REPEAT( PASS_UBA_OUT, BP_FIFO_COUNT ) int_cba_buf_x, int_cba_buf_y, int_cba_buf_z );
#elif BP_BYPASS == 1
        static_assert( len_d == 1, "if BP_BYPASS is enabled, len_d must be 1" );
        static_assert( rr_unroll_factor % 3 == 0, "if BP_BYPASS is enabled, rr_unroll_factor must be a multiple of 3" );
        static_assert( ord_rys * nxyz % rr_unroll_factor == 0, "if BP_BYPASS is enabled, ord_rys * nxyz must be a multiple of rr_unroll_factor" );
        for( int i = 0; i < qcf::util::ceil_div( ord_rys * nxyz, rr_unroll_factor ); ++i )
COND_REWIND_PIPELINE_II(1)
        {
            const ap_uint<ucba_pt::bit_width / 4>& ucba_0 = ucba_stream_in_0.read();
            const ap_uint<ucba_pt::bit_width / 4>& ucba_1 = ucba_stream_in_1.read();
            const ap_uint<ucba_pt::bit_width / 4>& ucba_2 = ucba_stream_in_2.read();
            const ap_uint<ucba_pt::bit_width / 4>& ucba_3 = ucba_stream_in_3.read();
            ucba_stream_out_0.write( ucba_0 );
            ucba_stream_out_1.write( ucba_1 );
            ucba_stream_out_2.write( ucba_2 );
            ucba_stream_out_3.write( ucba_3 );
            ucba_pt ucba = (D_WIDTH( ucba_pt::bit_width )) ((D_WIDTH( ucba_pt::bit_width / 2 )) (ucba_3, ucba_2), (D_WIDTH( ucba_pt::bit_width / 2 )) (ucba_1, ucba_0));

            for( int u = 0; u < rr_unroll_factor / 3; ++u )
                for( int c = 0; c < len_c; ++c )
                    for( int b = 0; b < len_b; ++b )
                        for( int a = 0; a < len_a; ++a )
                        {
//                            printf("x[%d][%d][%d][%d]: %f\n", i * (rr_unroll_factor / 3) + u, c, b, a, (fp_t) ucba[u * 3 + 0][c][b][a]);
//                            printf("y[%d][%d][%d][%d]: %f\n", i * (rr_unroll_factor / 3) + u, c, b, a, (fp_t) ucba[u * 3 + 1][c][b][a]);
//                            printf("z[%d][%d][%d][%d]: %f\n", i * (rr_unroll_factor / 3) + u, c, b, a, (fp_t) ucba[u * 3 + 2][c][b][a]);
                            int_cba_buf_x[i * (rr_unroll_factor / 3) + u][c][b][a] = ucba[u * 3 + 0][c][b][a];
                            int_cba_buf_y[i * (rr_unroll_factor / 3) + u][c][b][a] = ucba[u * 3 + 1][c][b][a];
                            int_cba_buf_z[i * (rr_unroll_factor / 3) + u][c][b][a] = ucba[u * 3 + 2][c][b][a];
                        }
        }
#endif
        gaussian_quadrature_split<rys_unroll, len_d, len_c, ncg_d, ncg_c, ncg_b, ncg_a, 0, ncg_b_gq_a>( int_cba_buf_x, int_cba_buf_y, int_cba_buf_z, partial_eris_stream_out_0, partial_eris_stream_out_1 );
    }
}
}
