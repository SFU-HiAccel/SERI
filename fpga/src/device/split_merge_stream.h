#ifndef QC_FPGA_SPLIT_MERGE_STREAM_H
#define QC_FPGA_SPLIT_MERGE_STREAM_H

#include "streams.h"

template<int depth, int bit_width>
void split_stream(
        hls::stream<D_WIDTH( bit_width ) >& fw_stream_in,
        qcf::ostream<D_WIDTH( bit_width / 2 ) >& split_hw_stream_out_0,
        qcf::ostream<D_WIDTH( bit_width / 2 ) >& split_hw_stream_out_1
)
{
    AGGRESSIVE_UNROLL_COND_INLINE

    SPLIT_STREAM_LOOP:
    for( int i = 0; i < depth; ++i )
#pragma hls loop_flatten
COND_REWIND_PIPELINE_II(1)
    {
        D_WIDTH( bit_width ) result = fw_stream_in.read();
        split_hw_stream_out_0.write( result.range( bit_width / 2 - 1, 0 ) );
        split_hw_stream_out_1.write( result.range( bit_width - 1, bit_width / 2 ) );
    }
}

template<int depth, int bit_width>
void merge_stream(
        qcf::istream<D_WIDTH( bit_width / 2 ) >& split_hw_stream_in_0,
        qcf::istream<D_WIDTH( bit_width / 2 ) >& split_hw_stream_in_1,
        hls::stream<D_WIDTH( bit_width ) >& fw_stream_out
)
{
    AGGRESSIVE_UNROLL_COND_INLINE

    SPLIT_STREAM_LOOP:
    for( int i = 0; i < depth; ++i )
#pragma hls loop_flatten
COND_REWIND_PIPELINE_II(1)
    {
        D_WIDTH( bit_width / 2 ) packet_0 = split_hw_stream_in_0.read();
        D_WIDTH( bit_width / 2 ) packet_1 = split_hw_stream_in_1.read();
        fw_stream_out.write( (D_WIDTH( bit_width )) (packet_1, packet_0) );
    }
}

#endif //QC_FPGA_SPLIT_MERGE_STREAM_H
