#include "internal_types.h"
#include "recurrence_relations.h"
#include "hls_stream.h"

using namespace qcf;


extern "C" {
void k_recurrence_relations(
        hls::stream<D_WIDTH( xyz_derived_pt::bit_width ) >& xyz_derived_stream_in,
        hls::stream<D_WIDTH( rys_wt_pt::bit_width ) >& rys_wt_stream_in,
        hls::stream<D_WIDTH( tb_b_pt::bit_width ) >& tb_b_stream_in,
        hls::stream<D_WIDTH( tb_c_pt::bit_width ) >& tb_c_stream_in,
#if BP_BYPASS == 0
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 2 ) >& ucba_stream_out_0,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 2 ) >& ucba_stream_out_1,
#elif BP_BYPASS == 1
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_0,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_1,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_2,
        qcf::ostream<D_WIDTH( ucba_pt::bit_width / 4 ) >& ucba_stream_out_3,
#endif
        hls::stream<n_t>& n_stream_in,
        hls::stream<n_t>& n_stream_out
)
{
#pragma hls interface ap_ctrl_none port=return

    n_t n = n_stream_in.read();
    n_stream_out.write( n );

    recurrence_relations<am_b, am_d, am_n, am_m, len_a, len_b, len_c, len_d, len_n, len_m, ord_rys>( xyz_derived_stream_in, rys_wt_stream_in, tb_b_stream_in, tb_c_stream_in,
#if BP_BYPASS == 0
                                                                                                     ucba_stream_out_0, ucba_stream_out_1,
#elif BP_BYPASS == 1
                                                                                                     ucba_stream_out_0, ucba_stream_out_1, ucba_stream_out_2, ucba_stream_out_3,
#endif
                                                                                                     n );
}
}