#ifndef QC_FPGA_BUFFER_PERMUTATION_H
#define QC_FPGA_BUFFER_PERMUTATION_H

#include "common/types.h"
#include "common/repeat.h"
#include "common/parameters.h"
#include "streams.h"
#include <hls_stream.h>
#include "internal_types.h"
#include "packed_array.hpp"
#include <algorithm>
#include <cmath>

#define ARGS_PKT_FIFOS( z ) hls::stream<ucba_pt>& pkt_fifos_##z,

template<int bp_d, int bp_phi, int bp_n>
void load_fifos(
        REPEAT( ARGS_PKT_FIFOS, BP_FIFO_COUNT )
        qcf::istream<ap_uint<ucba_pt::bit_width>>& dcbaox_stream_in
)
{
    AGGRESSIVE_UNROLL_COND_INLINE

    constexpr int load_fifos_ii = bp_n * bp_d;

    // bp_phi is total packets, bp_phi_n is max packets per fifo, rr_unroll_factor is num ubas per packet
    LOAD_BP_FIFOS:
    for( int phi_n = 0; phi_n < qcf::util::ceil_div( bp_phi, bp_n ); ++phi_n )
COND_REWIND_PIPELINE_II(load_fifos_ii)
    {
#define BP_LOAD_LOOP( z ) if( phi_n * bp_n + z < bp_phi ) { for( int d = 0; d < bp_d; ++d ) pkt_fifos_##z.write( dcbaox_stream_in.read() ); }
        REPEAT( BP_LOAD_LOOP, BP_FIFO_COUNT );
    }
}

template<int bp_d, int bp_phi, int len_c, int bp_n>
void flush_fifos(
        REPEAT( ARGS_UBA_OUT, BP_FIFO_COUNT )
        REPEAT( ARGS_PKT_FIFOS, BP_FIFO_COUNT )
        int x
)
{
    AGGRESSIVE_UNROLL_COND_INLINE

    DEF_UBA_OUT_ARR( BP_FIFO_COUNT );

    LOAD_CBA_PACKETS_TO_BUF:
    for( int phi_n = 0; phi_n < qcf::util::ceil_div( bp_phi, bp_n ); ++phi_n ) // bp_phi is total packets, bp_phi_n is max packets per fifo, rr_unroll_factor is num ubas per packet
    {
        for( int d = 0; d < bp_d; ++d )
#pragma hls loop_flatten
COND_REWIND_PIPELINE_II(len_c)
        {
            ucba_pt ba_pkts[bp_n];
#pragma hls array_partition variable=ba_pkts complete dim=0
#pragma hls aggregate variable=ba_pkts

#define BP_READ( z ) if( phi_n * bp_n + z < bp_phi ) ba_pkts[z] = pkt_fifos_##z.read();
            REPEAT(BP_READ, BP_FIFO_COUNT)

            for( int c = 0; c < len_c; ++c )
//#pragma hls loop_flatten
//#pragma hls pipeline II=1
            {
                for( int n = 0; n < bp_n; ++n )
                {
                    if( phi_n * bp_n + n >= bp_phi ) break;

                    uba_pt internal_uba_packet;
                    for( int u = 0; u < rr_unroll_factor; ++u )
                    {
                        for( int b = 0; b < len_b; ++b )
                        {
                            for( int a = 0; a < len_a; ++a )
                            {
                                internal_uba_packet[u][b][a] = ba_pkts[n][u][c][b][a];
                                internal_uba_packet[u][b][a] = ba_pkts[n][u][c][b][a];
                                internal_uba_packet[u][b][a] = ba_pkts[n][u][c][b][a];
                            }
                        }
                    }
                    uba_stream_out[n]->write( internal_uba_packet );
                }
            }
        }
    }
}

#endif //QC_FPGA_BUFFER_PERMUTATION_H
