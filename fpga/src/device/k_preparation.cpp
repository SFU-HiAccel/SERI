#include "internal_types.h"
#include "preparation.h"
#include "hls_stream.h"

using namespace qcf;

extern "C" {
void k_preparation(
        const qp_t* abcd_inp,
        const rys_t* rtwt_rys_inp,
        int n,
        hls::stream<D_WIDTH( xyz_derived_pt::bit_width ) >& xyz_derived_stream_out,
        hls::stream<D_WIDTH( rys_wt_pt::bit_width ) >& rys_wt_stream_out,
        hls::stream<D_WIDTH( tb_b_pt::bit_width ) >& tb_b_stream_out,
        hls::stream<D_WIDTH( tb_c_pt::bit_width ) >& tb_c_stream_out,
        hls::stream<n_t>& n_stream_out
)
{
#pragma hls interface mode=m_axi port=abcd_inp bundle=abcd_inp
#pragma hls interface mode=m_axi port=rtwt_rys_inp bundle=rtwt_rys_inp
#pragma hls aggregate variable=abcd_inp
#pragma hls aggregate variable=rtwt_rys_inp

    n_stream_out.write( n );

    for( int i = 0; i < n; ++i )
#pragma hls pipeline II=gq_std_tc
    {
        xyz_derived_pt xyz_derived; /* x, y, z derived for AB, CD, PA, QC, PQ */
        tb_b_pt tb_b;               /* auxiliary B array */
        tb_c_pt tb_c;               /* auxiliary C array */

        rys_t rtwt_rys = rtwt_rys_inp[i];
        qp_t abcd = abcd_inp[i];

        preparation( abcd, rtwt_rys, xyz_derived, tb_b, tb_c );

        rys_wt_pt rys_wt;
        rys_wt.copy_from( rtwt_rys.wt );

        xyz_derived_stream_out.write( xyz_derived );
        rys_wt_stream_out.write( rys_wt );
        tb_b_stream_out.write( tb_b );
        tb_c_stream_out.write( tb_c );
    }
}
}