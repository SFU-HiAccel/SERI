#include "internal_types.h"
#include <hls_stream.h>
#include "buffer_permutation.h"
#include "split_merge_stream.h"

using namespace qcf;

extern "C" {
void k_buffer_permutation(
        hls::stream<D_WIDTH( ucba_pt::bit_width / 2 ) >& ucba_stream_in_0,
        hls::stream<D_WIDTH( ucba_pt::bit_width / 2 ) >& ucba_stream_in_1,
        REPEAT( ARGS_UBA_OUT, BP_FIFO_COUNT )
        hls::stream<n_t>& n_stream_in,
        hls::stream<n_t>& n_stream_out
)
{
#pragma hls interface ap_ctrl_none port=return

    constexpr int cba_fifo_depth = qcf::util::ceil_div( bp_phi, bp_fifo_count ) * len_d;
// last fifo doesn't need to hold data as it is consumed right away in dataflow
    hls::stream<D_WIDTH( ucba_pt::bit_width ) > merged_stream;
#if AM_ABCD == 3230 || AM_ABCD == 3231
#define DEF_PKT_FIFOS( z ) hls::stream<ucba_pt> pkt_fifos_##z; \
                           PRAGMA( hls stream variable=pkt_fifos_##z type=fifo depth=cba_fifo_depth )
#else
#define DEF_PKT_FIFOS( z ) hls::stream<ucba_pt> pkt_fifos_##z; \
                           PRAGMA( hls stream variable=pkt_fifos_##z type=fifo depth=cba_fifo_depth ) \
                           PRAGMA( hls bind_storage variable=pkt_fifos_##z type=fifo impl=bram )
#endif
#define PASS_PKT_FIFOS( z ) pkt_fifos_##z,
    REPEAT( DEF_PKT_FIFOS, BP_FIFO_COUNT )

    n_t n = n_stream_in.read();
    n_stream_out.write( n );

    for( int i_n = 0; i_n < n; ++i_n )
    AGGRESSIVE_UNROLL_COND_DATAFLOW_PIPELINE
    {
        merge_stream<bp_phi * len_d, ucba_pt::bit_width>( ucba_stream_in_0, ucba_stream_in_1, merged_stream );
        load_fifos<bp_d, bp_phi, bp_fifo_count>( REPEAT( PASS_PKT_FIFOS, BP_FIFO_COUNT ) merged_stream );
        flush_fifos<bp_d, bp_phi, len_c, bp_fifo_count>( REPEAT( PASS_UBA_OUT, BP_FIFO_COUNT ) REPEAT( PASS_PKT_FIFOS, BP_FIFO_COUNT ) 0 );
    }
}
}
