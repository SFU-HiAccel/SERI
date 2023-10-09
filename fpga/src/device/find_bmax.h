#ifndef QC_FPGA_FIND_BMAX_H
#define QC_FPGA_FIND_BMAX_H

#include "common/types.h"
#include "common/repeat.h"
#include "common/parameters.h"
#include "streams.h"
#include <hls_stream.h>
#include "internal_types.h"
#include "packed_array.hpp"
#include <algorithm>
#include <cmath>

#if AM_ABCD == 3333
template<int ncg_d, int ncg_c>
void find_b_max_passthrough(
        hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_in_0,
        hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_in_1,
        qcf::ostream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_out_0,
        qcf::ostream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_out_1,
        qcf::ostream<fp_t>& b_max_stream_out
)
{
    AGGRESSIVE_UNROLL_COND_INLINE

    fp_t b_max = 0;

    ND_NC_LOOP:
    for( int i = 0; i < ncg_d * ncg_c; i++ )
#pragma hls loop_flatten
            COND_REWIND_PIPELINE_II( 1 )
    {
        fp_t local_b_max = 0;
        D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) packet_0 = eris_stream_in_0.read();
        D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) packet_1 = eris_stream_in_1.read();
        ba_cart_eris_pt packet = (D_WIDTH( ba_cart_eris_pt::bit_width )) (packet_1, packet_0);

        for( int j = 0; j < ncg_b * ncg_a; ++j )
            local_b_max = std::max( local_b_max, std::fabs( packet[0][j] ) );
        b_max = std::max( b_max, local_b_max );

        eris_stream_out_0.write( packet_0 );
        eris_stream_out_1.write( packet_1 );

        if( i + 1 == ncg_d * ncg_c )
        {
            b_max_stream_out.write( b_max );
            b_max = 0;
        }
    }
}

#else

#if GQ_KERNEL_SPLIT == 2
template<int ncg_d, int ncg_c>
void find_b_max_passthrough(
        hls::stream<D_WIDTH( ba_cart_eris_a_pt::bit_width / 2 ) >& partial_eris_a_stream_in_0,
        hls::stream<D_WIDTH( ba_cart_eris_a_pt::bit_width / 2 ) >& partial_eris_a_stream_in_1,
        hls::stream<D_WIDTH( ba_cart_eris_b_pt::bit_width / 2 ) >& partial_eris_b_stream_in_0,
        hls::stream<D_WIDTH( ba_cart_eris_b_pt::bit_width / 2 ) >& partial_eris_b_stream_in_1,
        qcf::ostream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_out_0,
        qcf::ostream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_out_1,
        qcf::ostream<fp_t>& b_max_stream_out,
        n_t n
)
{
#pragma hls inline

    fp_t b_max = 0;

    N_LOOP:
    for( int i_n = 0; i_n < n; ++i_n )
    {
        ND_NC_LOOP:
        for( int i = 0; i < ncg_d * ncg_c; i++ ) // always frp to prevent deadlocks
#pragma hls loop_flatten
#ifdef NO_FRP_FB
#pragma hls pipeline II=1
#else
#pragma hls pipeline II=1 style=frp
#endif
        {
            fp_t local_b_max = 0;
            D_WIDTH( ba_cart_eris_pt::bit_width ) full = ((partial_eris_b_stream_in_1.read(), partial_eris_b_stream_in_0.read()), (partial_eris_a_stream_in_1.read(), partial_eris_a_stream_in_0.read()));
            D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) packet_0 = full.range( ba_cart_eris_pt::bit_width / 2 - 1, 0 );
            D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) packet_1 = full.range( ba_cart_eris_pt::bit_width - 1, ba_cart_eris_pt::bit_width / 2 );
            ba_cart_eris_pt packet = (D_WIDTH( ba_cart_eris_pt::bit_width )) (packet_1, packet_0);

            for( int j = 0; j < ncg_b * ncg_a; ++j )
                local_b_max = std::max( local_b_max, packet[0][j] >= 0 ? packet[0][j] : -packet[0][j] );
            b_max = std::max( b_max, local_b_max );

            eris_stream_out_0.write( packet_0 );
            eris_stream_out_1.write( packet_1 );

            if( i + 1 == ncg_d * ncg_c )
            {
                b_max_stream_out.write( b_max );
                b_max = 0;
            }
        }
    }
}
#elif GQ_KERNEL_SPLIT == 1


template<int ncg_d, int ncg_c>
void find_b_max_passthrough(
        hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_in_0,
        hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_in_1,
        qcf::ostream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_out_0,
        qcf::ostream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& eris_stream_out_1,
        qcf::ostream<fp_t>& b_max_stream_out,
        n_t n
)
{
#pragma hls inline

    fp_t b_max = 0;

    N_LOOP:
    for( int i_n = 0; i_n < n; ++i_n )
    {
        ND_NC_LOOP:
        for( int i = 0; i < ncg_d * ncg_c; i++ ) // always frp to prevent deadlocks
#pragma hls loop_flatten
#ifdef NO_FRP
#pragma hls pipeline II=1
#else
#pragma hls pipeline II=1 style=frp
#endif
        {
            fp_t local_b_max = 0;
            D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) packet_0 = eris_stream_in_0.read();
            D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) packet_1 = eris_stream_in_1.read();
            
	    // write out packets before concatenation, because for select quartet classes, concatention will actaully modify the ap_uint values in csim
            eris_stream_out_0.write( packet_0 );
            eris_stream_out_1.write( packet_1 );
            ba_cart_eris_pt packet = (D_WIDTH( ba_cart_eris_pt::bit_width )) (packet_1, packet_0);

            for( int j = 0; j < ncg_b * ncg_a; ++j )
                local_b_max = std::max( local_b_max, std::fabs( packet[0][j] ) );
            b_max = std::max( b_max, local_b_max );

            if( i + 1 == ncg_d * ncg_c )
            {
                b_max_stream_out.write( b_max );
                b_max = 0;
            }
        }
    }
}
#endif

#endif


#endif //QC_FPGA_FIND_BMAX_H
