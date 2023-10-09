#include "internal_types.h"
#include "hls_stream.h"
#include "find_bmax.h"

using namespace qcf;
using qcf::util::partial_array_t;
using qcf::util::gq_seg_index_t;

extern "C" {
void k_find_bmax(
#if GQ_KERNEL_SPLIT == 2
        hls::stream<D_WIDTH( ba_cart_eris_a_pt::bit_width / 2 ) >& partial_eris_a_stream_in_0,
        hls::stream<D_WIDTH( ba_cart_eris_a_pt::bit_width / 2 ) >& partial_eris_a_stream_in_1,
#endif
        hls::stream<D_WIDTH( ba_cart_eris_b_pt::bit_width / 2 ) >& partial_eris_b_stream_in_0,
        hls::stream<D_WIDTH( ba_cart_eris_b_pt::bit_width / 2 ) >& partial_eris_b_stream_in_1,
        hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& cart_eris_stream_out_0,
        hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) >& cart_eris_stream_out_1,
        hls::stream<fp_t>& b_max_stream_out,
        hls::stream<n_t>& n_stream_in
)
{
#pragma hls interface ap_ctrl_none port=return
    n_t n = n_stream_in.read();

#if AM_ABCD == 3333

    hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) > partial_eris_stream_in_0;
    hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) > partial_eris_stream_in_1;
    N_LOOP:
    for( int i_n = 0; i_n < n; ++i_n )
    AGGRESSIVE_UNROLL_COND_DATAFLOW_PIPELINE
    {
        for( int i = 0; i < ncg_d * ncg_c; i++ )
#pragma hls pipeline II=1
        {
            D_WIDTH( ba_cart_eris_pt::bit_width ) full = ((partial_eris_b_stream_in_1.read(), partial_eris_b_stream_in_0.read()), (partial_eris_a_stream_in_1.read(), partial_eris_a_stream_in_0.read()));
            partial_eris_stream_in_0.write( full.range( ba_cart_eris_pt::bit_width / 2 - 1, 0 ) );
            partial_eris_stream_in_1.write( full.range( ba_cart_eris_pt::bit_width - 1, ba_cart_eris_pt::bit_width / 2 ) );
        }

        find_b_max_passthrough<ncg_d, ncg_c>( partial_eris_stream_in_0, partial_eris_stream_in_1, cart_eris_stream_out_0, cart_eris_stream_out_1, b_max_stream_out);
    }
#else

#if GQ_KERNEL_SPLIT == 2
    find_b_max_passthrough<ncg_d, ncg_c>( partial_eris_a_stream_in_0, partial_eris_a_stream_in_1, partial_eris_b_stream_in_0, partial_eris_b_stream_in_1, cart_eris_stream_out_0, cart_eris_stream_out_1, b_max_stream_out, n );
#elif GQ_KERNEL_SPLIT == 1
    find_b_max_passthrough<ncg_d, ncg_c>( partial_eris_b_stream_in_0, partial_eris_b_stream_in_1, cart_eris_stream_out_0, cart_eris_stream_out_1, b_max_stream_out, n );
#endif
#endif
}
}
