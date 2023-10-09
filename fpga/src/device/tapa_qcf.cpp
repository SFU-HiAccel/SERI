#ifdef TAPA_FLOW

#include <cstdint>
#include <tapa.h>
#include "preparation.h"
#include "recurrence_relations.h"
#include "gaussian_qaudrature_opt.h"
#include "compress_store_split.h"

#define TAPA_ARGS_CART_ERIS( z ) tapa::mmap <packed_eri_t> cart_eris_##z,


void t_preparation(
        tapa::mmap<const qp_t> abcd_inp,
        tapa::mmap<const D_WIDTH( qcf::util::next_pow2(sizeof(rys_t) * 8 ))> rtwt_rys_inp,
        n_t n,
        tapa::ostream <D_WIDTH( xyz_derived_pt::bit_width )>& xyz_derived_stream_out,
        tapa::ostream <D_WIDTH( rys_wt_pt::bit_width )>& rys_wt_stream_out,
        tapa::ostream <D_WIDTH( tb_b_pt::bit_width )>& tb_b_stream_out,
        tapa::ostream <D_WIDTH( tb_c_pt::bit_width )>& tb_c_stream_out,
        tapa::ostream <n_t>& n_stream_out
)
{
#pragma hls aggregate variable=abcd_inp
#pragma hls aggregate variable=rtwt_rys_inp
    constexpr int target_ii = std::max( ncg_d * ncg_c, ord_rys * nxyz );

    n_stream_out.write( n );

    for( int i = 0; i < n; ++i )
#pragma hls pipeline II=target_ii
    {
        xyz_derived_pt xyz_derived; /* x, y, z derived for AB, CD, PA, QC, PQ */
        tb_b_pt tb_b;               /* auxiliary B array */
        tb_c_pt tb_c;               /* auxiliary C array */

        rys_t rtwt_rys = *reinterpret_cast<const rys_t*>(&rtwt_rys_inp[i]);
        preparation( abcd_inp[i], rtwt_rys, xyz_derived, tb_b, tb_c );

        rys_wt_pt rys_wt;
        rys_wt.copy_from( rtwt_rys.wt );

        xyz_derived_stream_out.write( xyz_derived );
        rys_wt_stream_out.write( rys_wt );
        tb_b_stream_out.write( tb_b );
        tb_c_stream_out.write( tb_c );
    }
}

void t_recurrence_relations(
        tapa::istream <D_WIDTH( xyz_derived_pt::bit_width )>& xyz_derived_stream_in,
        tapa::istream <D_WIDTH( rys_wt_pt::bit_width )>& rys_wt_stream_in,
        tapa::istream <D_WIDTH( tb_b_pt::bit_width )>& tb_b_stream_in,
        tapa::istream <D_WIDTH( tb_c_pt::bit_width )>& tb_c_stream_in,
        tapa::ostream <D_WIDTH( cba_pt::bit_width )>& dcbaox_stream_out,
        tapa::istream <n_t>& n_stream_in,
        tapa::ostream <n_t>& n_stream_out
)
{
    n_t n = n_stream_in.read();
    n_stream_out.write( n );

    recurrence_relations<am_b, am_d, am_n, am_m, len_a, len_b, len_c, len_d, len_n, len_m, ord_rys>( xyz_derived_stream_in, rys_wt_stream_in, tb_b_stream_in, tb_c_stream_in, dcbaox_stream_out, n );
}

void t_permutation(
        tapa::istream<D_WIDTH( cba_pt::bit_width ) >& dcbaox_stream_in,
        tapa::ostream<D_WIDTH( ba_pt::bit_width ) >& ba_x_stream_out,
        tapa::ostream<D_WIDTH( ba_pt::bit_width ) >& ba_y_stream_out,
        tapa::ostream<D_WIDTH( ba_pt::bit_width ) >& ba_z_stream_out,
        tapa::istream<n_t>& n_stream_in,
        tapa::ostream<n_t>& n_stream_out
)
{
    constexpr int cba_fifo_depth = len_d * ord_rys;
// last fifo doesn't need to hold data as it is consumed right away in dataflow
    hls::stream<cba_pt> cba_x_fifo;
    hls::stream<cba_pt> cba_y_fifo;
    hls::stream<cba_pt> cba_z_fifo;
#pragma hls stream variable=cba_x_fifo type=fifo depth=cba_fifo_depth
#pragma hls stream variable=cba_y_fifo type=fifo depth=cba_fifo_depth
#pragma hls stream variable=cba_z_fifo type=fifo depth=cba_fifo_depth

    n_t n = n_stream_in.read();
    n_stream_out.write( n );

    for( int i_n = 0; i_n < n; ++i_n )
#pragma hls dataflow
    {
        load_xyz_streams<ord_rys, len_d>( dcbaox_stream_in, cba_x_fifo, cba_y_fifo, cba_z_fifo );
        flush_xyz_streams<ord_rys, len_d, len_c>( cba_x_fifo, cba_y_fifo, cba_z_fifo, ba_x_stream_out, ba_y_stream_out, ba_z_stream_out );
    }
}

void t_gaussian_quadrature_a(
        tapa::istream<D_WIDTH( ba_pt::bit_width ) >& ba_x_stream_in,
        tapa::istream<D_WIDTH( ba_pt::bit_width ) >& ba_y_stream_in,
        tapa::istream<D_WIDTH( ba_pt::bit_width ) >& ba_z_stream_in,
        tapa::ostream<D_WIDTH( ba_pt::bit_width ) >& ba_x_stream_out,
        tapa::ostream<D_WIDTH( ba_pt::bit_width ) >& ba_y_stream_out,
        tapa::ostream<D_WIDTH( ba_pt::bit_width ) >& ba_z_stream_out,
        tapa::ostream<ap_uint<ba_cart_eris_a_pt::bit_width / 2>>& partial_eris_stream_out_0,
        tapa::ostream<ap_uint<ba_cart_eris_a_pt::bit_width / 2>>& partial_eris_stream_out_1,
        tapa::istream<n_t>& n_stream_in,
        tapa::ostream<n_t>& n_stream_out
)
{
    constexpr int rys_unroll = ord_rys;

    hls::stream<D_WIDTH( ba_cart_eris_a_pt::bit_width ) > eris_stream_out;

    n_t n = n_stream_in.read();
    n_stream_out.write( n );

    for( int i_n = 0; i_n < n; ++i_n )
#pragma hls dataflow
    {
        ba_pt int_cba_buf_x[rys_unroll][len_d * len_c];
        ba_pt int_cba_buf_y[rys_unroll][len_d * len_c];
        ba_pt int_cba_buf_z[rys_unroll][len_d * len_c];
#pragma hls array_partition variable=int_cba_buf_x complete dim=1
#pragma hls array_partition variable=int_cba_buf_y complete dim=1
#pragma hls array_partition variable=int_cba_buf_z complete dim=1
#pragma hls aggregate variable=int_cba_buf_x
#pragma hls aggregate variable=int_cba_buf_y
#pragma hls aggregate variable=int_cba_buf_z
#pragma hls bind_storage variable=int_cba_buf_x type=ram_2p impl=bram
#pragma hls bind_storage variable=int_cba_buf_y type=ram_2p impl=bram
#pragma hls bind_storage variable=int_cba_buf_z type=ram_2p impl=bram
#pragma hls stream variable=int_cba_buf_x type=pipo depth=2
#pragma hls stream variable=int_cba_buf_y type=pipo depth=2
#pragma hls stream variable=int_cba_buf_z type=pipo depth=2

        load_dcbaox_bufs_pt<rys_unroll, len_d, len_c>( ba_x_stream_in, ba_y_stream_in, ba_z_stream_in, int_cba_buf_x, int_cba_buf_y, int_cba_buf_z, ba_x_stream_out, ba_y_stream_out, ba_z_stream_out );
        gaussian_quadrature_split<rys_unroll, len_d, len_c, ncg_d, ncg_c, ncg_b, ncg_a, 0, ncg_b_gq_a>( int_cba_buf_x, int_cba_buf_y, int_cba_buf_z, eris_stream_out );
        split_stream<ncg_d * ncg_c, ba_cart_eris_a_pt::bit_width>( eris_stream_out, partial_eris_stream_out_0, partial_eris_stream_out_1 );
    }
}

void t_gaussian_quadrature_b(
        tapa::istream<D_WIDTH( ba_pt::bit_width ) >& ba_x_stream_in,
        tapa::istream<D_WIDTH( ba_pt::bit_width ) >& ba_y_stream_in,
        tapa::istream<D_WIDTH( ba_pt::bit_width ) >& ba_z_stream_in,
        tapa::ostream<ap_uint<ba_cart_eris_b_pt::bit_width / 2>>& partial_eris_stream_out_0,
        tapa::ostream<ap_uint<ba_cart_eris_b_pt::bit_width / 2>>& partial_eris_stream_out_1,
        tapa::istream<n_t>& n_stream_in,
        tapa::ostream<n_t>& n_stream_out
)
{
    constexpr int rys_unroll = ord_rys;

    hls::stream<D_WIDTH( ba_cart_eris_b_pt::bit_width ) > eris_stream_out;

    n_t n = n_stream_in.read();
    n_stream_out.write( n );

    for( int i_n = 0; i_n < n; ++i_n )
#pragma hls dataflow
    {
        ba_pt int_cba_buf_x[rys_unroll][len_d * len_c];
        ba_pt int_cba_buf_y[rys_unroll][len_d * len_c];
        ba_pt int_cba_buf_z[rys_unroll][len_d * len_c];
#pragma hls array_partition variable=int_cba_buf_x complete dim=1
#pragma hls array_partition variable=int_cba_buf_y complete dim=1
#pragma hls array_partition variable=int_cba_buf_z complete dim=1
#pragma hls aggregate variable=int_cba_buf_x
#pragma hls aggregate variable=int_cba_buf_y
#pragma hls aggregate variable=int_cba_buf_z
#pragma hls bind_storage variable=int_cba_buf_x type=ram_2p impl=bram
#pragma hls bind_storage variable=int_cba_buf_y type=ram_2p impl=bram
#pragma hls bind_storage variable=int_cba_buf_z type=ram_2p impl=bram
#pragma hls stream variable=int_cba_buf_x type=pipo depth=2
#pragma hls stream variable=int_cba_buf_y type=pipo depth=2
#pragma hls stream variable=int_cba_buf_z type=pipo depth=2

        load_dcbaox_bufs<rys_unroll, len_d, len_c>( ba_x_stream_in, ba_y_stream_in, ba_z_stream_in, int_cba_buf_x, int_cba_buf_y, int_cba_buf_z );
        gaussian_quadrature_split<rys_unroll, len_d, len_c, ncg_d, ncg_c, ncg_b, ncg_a, ncg_b_gq_a, ncg_b>( int_cba_buf_x, int_cba_buf_y, int_cba_buf_z, eris_stream_out );
        split_stream<ncg_d * ncg_c, ba_cart_eris_b_pt::bit_width>( eris_stream_out, partial_eris_stream_out_0, partial_eris_stream_out_1 );
    }
}

void t_find_bmax(
        tapa::istream <ap_uint<ba_cart_eris_a_pt::bit_width / 2>>& partial_eris_a_stream_in_0,
        tapa::istream <ap_uint<ba_cart_eris_a_pt::bit_width / 2>>& partial_eris_a_stream_in_1,
        tapa::istream <ap_uint<ba_cart_eris_b_pt::bit_width / 2>>& partial_eris_b_stream_in_0,
        tapa::istream <ap_uint<ba_cart_eris_b_pt::bit_width / 2>>& partial_eris_b_stream_in_1,
        tapa::ostream <D_WIDTH( ba_cart_eris_pt::bit_width / 2 )>& partial_eris_stream_out_0,
        tapa::ostream <D_WIDTH( ba_cart_eris_pt::bit_width / 2 )>& partial_eris_stream_out_1,
        tapa::ostream <fp_t>& b_max_stream_out,
        tapa::istream <n_t>& n_stream_in
)
{
    n_t n = n_stream_in.read();

    hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) > partial_eris_stream_in_0;
    hls::stream<D_WIDTH( ba_cart_eris_pt::bit_width / 2 ) > partial_eris_stream_in_1;

    N_LOOP:
    for( int i_n = 0; i_n < n; ++i_n )
#pragma hls dataflow
    {
        for( int i = 0; i < ncg_d * ncg_c; i++ )
        {
            D_WIDTH( ba_cart_eris_pt::bit_width ) full = ((partial_eris_b_stream_in_1.read(), partial_eris_b_stream_in_0.read()), (partial_eris_a_stream_in_1.read(), partial_eris_a_stream_in_0.read()));
            partial_eris_stream_in_0.write( full.range( ba_cart_eris_pt::bit_width / 2 - 1, 0 ) );
            partial_eris_stream_in_1.write( full.range( ba_cart_eris_pt::bit_width - 1, ba_cart_eris_pt::bit_width / 2 ) );
        }

        find_b_max_passthrough<ncg_d, ncg_c>( partial_eris_stream_in_0, partial_eris_stream_in_1, partial_eris_stream_out_0, partial_eris_stream_out_1, b_max_stream_out, n );
    }
}

void t_compress_store(
        tapa::istream <D_WIDTH( ba_cart_eris_pt::bit_width / 2 )>& cart_eris_stream_in_0,
        tapa::istream <D_WIDTH( ba_cart_eris_pt::bit_width / 2 )>& cart_eris_stream_in_1,
        tapa::istream <fp_t>& b_max_stream_in,
        REPEAT( TAPA_ARGS_CART_ERIS, NUM_ERIS_PORTS )
        n_t n
)
{

    fp_t cart_eris_buf[num_eris_ports * pack_count];
#pragma hls array_partition variable=cart_eris_buf complete dim=0

    fp_t e1;
    fp_t epsilon;

    N_LOOP:
    for( int i_n = 0; i_n < n; ++i_n )
    {
        STREAM_TO_ERI_PORTS:
        for( int i = 0; i < ncg_d * ncg_c; i++ )
#pragma hls loop_flatten
#pragma hls pipeline II=1
        {
            if( i == 0 )
                e1 = calc_epsilon_inv( b_max_stream_in.read(), &epsilon );

            ba_cart_eris_pt packet = (D_WIDTH( ba_cart_eris_pt::bit_width )) (cart_eris_stream_in_1.read(), cart_eris_stream_in_0.read());
            for( int j = 0; j < ncg_b * ncg_a; ++j )
                cart_eris_buf[j] = packet[0][j];

#define ASSIGN_CART_ERIS( z ) cart_eris_##z[i_n * ncg_d * ncg_c + i] = pack_and_compress_eri<pack_count>( &cart_eris_buf[z * pack_count], e1 );
#define REPEAT_ASSIGN_CART_ERIS( n ) REPEAT(ASSIGN_CART_ERIS, n )
            FOR_LAST( REPEAT_ASSIGN_CART_ERIS, NUM_ERIS_PORTS )

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


void qcf_top(
        tapa::mmap<const qp_t> abcd_inp,
        tapa::mmap<const D_WIDTH( qcf::util::next_pow2(sizeof(rys_t) * 8 ))> rtwt_rys_inp,
        // manually define not in macro cause TAPA is dumb
#if NUM_ERIS_PORTS >= 1
        tapa::mmap <packed_eri_t> cart_eris_0,
#endif
#if NUM_ERIS_PORTS >= 2
        tapa::mmap <packed_eri_t> cart_eris_1,
#endif
#if NUM_ERIS_PORTS >= 3
        tapa::mmap <packed_eri_t> cart_eris_2,
#endif
#if NUM_ERIS_PORTS >= 4
        tapa::mmap <packed_eri_t> cart_eris_3,
#endif
#if NUM_ERIS_PORTS >= 5
        tapa::mmap <packed_eri_t> cart_eris_4,
#endif
#if NUM_ERIS_PORTS >= 6
        tapa::mmap <packed_eri_t> cart_eris_5,
#endif
#if NUM_ERIS_PORTS >= 7
        tapa::mmap <packed_eri_t> cart_eris_6,
#endif
        n_t n
)
{
    tapa::stream <D_WIDTH( xyz_derived_pt::bit_width )> xyz_derived_stream( "xyz_derived_stream" );
    tapa::stream <D_WIDTH( rys_wt_pt::bit_width )> rys_wt_stream( "rys_wt_stream" );
    tapa::stream <D_WIDTH( tb_b_pt::bit_width )> tb_b_stream( "tb_b_stream" );
    tapa::stream <D_WIDTH( tb_c_pt::bit_width )> tb_c_stream( "tb_c_stream" );
    tapa::stream <D_WIDTH( cba_pt::bit_width )> dcbaox_stream( "dcbaox_stream" );
    tapa::stream <D_WIDTH( ba_pt::bit_width )> ba_x_stream_0("ba_x_stream_0");
    tapa::stream <D_WIDTH( ba_pt::bit_width )> ba_y_stream_0("ba_y_stream_0");
    tapa::stream <D_WIDTH( ba_pt::bit_width )> ba_z_stream_0("ba_z_stream_0");
    tapa::stream <D_WIDTH( ba_pt::bit_width )> ba_x_stream_1("ba_x_stream_1");
    tapa::stream <D_WIDTH( ba_pt::bit_width )> ba_y_stream_1("ba_y_stream_1");
    tapa::stream <D_WIDTH( ba_pt::bit_width )> ba_z_stream_1("ba_z_stream_1");
    tapa::stream <ap_uint<ba_cart_eris_a_pt::bit_width / 2>> partial_eris_stream_a_0( "partial_eris_stream_a_0" );
    tapa::stream <ap_uint<ba_cart_eris_a_pt::bit_width / 2>> partial_eris_stream_a_1( "partial_eris_stream_a_1" );
    tapa::stream <ap_uint<ba_cart_eris_b_pt::bit_width / 2>> partial_eris_stream_b_0( "partial_eris_stream_b_0" );
    tapa::stream <ap_uint<ba_cart_eris_b_pt::bit_width / 2>> partial_eris_stream_b_1( "partial_eris_stream_b_1" );
    tapa::stream <D_WIDTH( ba_cart_eris_pt::bit_width / 2 )> eris_stream_0( "eris_stream_0" );
    tapa::stream <D_WIDTH( ba_cart_eris_pt::bit_width / 2 )> eris_stream_1( "eris_stream_1" );
    tapa::stream <fp_t> b_max_stream( "b_max_stream" );
    tapa::stream <n_t> n_max_stream_0( "n_stream_0" );
    tapa::stream <n_t> n_max_stream_1( "n_stream_1" );
    tapa::stream <n_t> n_max_stream_2( "n_stream_2" );
    tapa::stream <n_t> n_max_stream_3( "n_stream_3" );
    tapa::stream <n_t> n_max_stream_4( "n_stream_4" );

    tapa::task()
            .invoke( t_preparation, abcd_inp, rtwt_rys_inp, n, xyz_derived_stream, rys_wt_stream, tb_b_stream, tb_c_stream, n_max_stream_0 )
            .invoke( t_recurrence_relations, xyz_derived_stream, rys_wt_stream, tb_b_stream, tb_c_stream, dcbaox_stream, n_max_stream_0, n_max_stream_1 )
            .invoke( t_permutation, dcbaox_stream, ba_x_stream_0, ba_y_stream_0, ba_z_stream_0, n_max_stream_1, n_max_stream_2 )
            .invoke( t_gaussian_quadrature_a, ba_x_stream_0, ba_y_stream_0, ba_z_stream_0, ba_x_stream_1, ba_y_stream_1, ba_z_stream_1, partial_eris_stream_a_0, partial_eris_stream_a_1, n_max_stream_2, n_max_stream_3 )
            .invoke( t_gaussian_quadrature_b, ba_x_stream_1, ba_y_stream_1, ba_z_stream_1, partial_eris_stream_b_0, partial_eris_stream_b_1, n_max_stream_3, n_max_stream_4 )
            .invoke( t_find_bmax, partial_eris_stream_a_0, partial_eris_stream_a_1, partial_eris_stream_b_0, partial_eris_stream_b_1, eris_stream_0, eris_stream_1, b_max_stream, n_max_stream_4 )
            .invoke( t_compress_store, eris_stream_0, eris_stream_1, b_max_stream, REPEAT( PASS_CART_ERIS, NUM_ERIS_PORTS ) n );
}

#endif