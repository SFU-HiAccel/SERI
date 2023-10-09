#include "internal_types.h"
#include "compress_store_split.h"
#include "hls_stream.h"
#include "packed_array.hpp"

using namespace qcf;

extern "C" {
void k_compress_store(
        hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& cart_eris_stream_in_0,
        hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& cart_eris_stream_in_1,
        hls::stream<fp_t>& b_max_stream_in,
        REPEAT( ARGS_CART_ERIS, NUM_ERIS_PORTS )
        int n
)
{
#define INTERFACE_CART_ERIS( z ) PRAGMA( hls interface mode=m_axi port=cart_eris_##z bundle=ce##z max_widen_bitwidth=256 max_write_burst_length=128 )
    REPEAT( INTERFACE_CART_ERIS, NUM_ERIS_PORTS )

    fp_t cart_eris_buf[num_eris_ports * pack_count];
#pragma hls array_partition variable=cart_eris_buf complete dim=0

#ifndef NO_FRP_CS
    fp_t e1, epsilon;
#endif
    N_LOOP:
    for( int i_n = 0; i_n < n; ++i_n )
    AGGRESSIVE_UNROLL_COND_PIPELINE
    {
#ifdef NO_FRP_CS
        fp_t e1, epsilon;
#endif
        STREAM_TO_ERI_PORTS:
        for( int i = 0; i < ncg_d * ncg_c; i++ )
#pragma hls loop_flatten
#ifdef NO_FRP_CS
#pragma hls pipeline II=1
#else
#pragma hls pipeline II=1 style=frp
#endif
        {
            if( i == 0 )
                e1 = calc_epsilon_inv( b_max_stream_in.read(), &epsilon );

            // type conversion needs to be split into 2 lines due to sw_emu bug
            auto uint = (D_WIDTH( ba_cart_eris_pt::bit_width )) (cart_eris_stream_in_1.read(), cart_eris_stream_in_0.read());
	    ba_cart_eris_pt packet = uint;
            for( int j = 0; j < ncg_b * ncg_a; ++j )
                cart_eris_buf[j] = packet[0][j];

#if NUM_ERIS_PORTS > 1
#define ASSIGN_CART_ERIS( z ) cart_eris_##z[i_n * ncg_d * ncg_c + i] = pack_and_compress_eri<pack_count>( &cart_eris_buf[z * pack_count], e1 );
#define REPEAT_ASSIGN_CART_ERIS( n ) REPEAT(ASSIGN_CART_ERIS, n )
            FOR_LAST( REPEAT_ASSIGN_CART_ERIS, NUM_ERIS_PORTS )
#endif

            // duplicate
            constexpr int past_last_eri_index = (ncg_b * ncg_a) - ((num_eris_ports - 1) * pack_count);
#define LAST_PACK_CART_ERIS( z ) packed_eri_t last_pack = pack_and_compress_eri<past_last_eri_index>( &cart_eris_buf[z * pack_count], e1 );
            FOR_LAST( LAST_PACK_CART_ERIS, NUM_ERIS_PORTS )
#pragma hls aggregate variable=last_pack
            memcpy( &last_pack.data[past_last_eri_index], &epsilon, sizeof( fp_t ) );
#define LAST_ASSIGN_CART_ERIS( z ) cart_eris_##z[i_n * ncg_d * ncg_c + i] = last_pack;
            FOR_LAST( LAST_ASSIGN_CART_ERIS, NUM_ERIS_PORTS )
        }
    }
}
}
