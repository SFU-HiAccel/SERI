#include "common/param_config.h"
#include "auto_cfg/connectivity.h"

using std::string;
using std::to_string;


int main( int argc, char* argv[] )
{
    if( argc != 4 )
        throw std::invalid_argument( "usage: " + string( argv[0] ) + " append_to.cfg append_to.tcl" );

    system_connectivity sc( argv[1], argv[2] );
    int multiplicity = atoi( argv[3] );

    constexpr bool use_alt = AM_ABCD != 3333 && AM_ABCD != 3332 && AM_ABCD != 3311 && AM_ABCD != 3310;

    pblock pb_bot_left = pblock( "pblock_BL" ).slr( 0 );
    pblock pb_bot_left_alt = pblock( "pblock_BL_alt" ).slr( 0 );
    pblock pb_bot_right = pblock( "pblock_BR" ).slr( 0 );
    pblock pb_mid_left = pblock( "pblock_ML" ).slr( 1 );
    pblock pb_mid_right = pblock( "pblock_MR" ).slr( 1 );

    kernel k_prep = sc.def_kernel( "k_preparation" ).num( multiplicity );
    kernel k_rr = RR_KERNEL_SPLIT != 2 ? sc.def_kernel( "k_recurrence_relations" ).num( multiplicity ) : kernel::null();
    kernel k_rr_a = RR_KERNEL_SPLIT == 2 ? sc.def_kernel( "k_rr_a" ).num( multiplicity ) : kernel::null();
    kernel k_rr_b = RR_KERNEL_SPLIT == 2 ? sc.def_kernel( "k_rr_b" ).num( multiplicity ) : kernel::null();
    kernel k_bp = (BP_BYPASS == 0) ? sc.def_kernel( "k_buffer_permutation" ).num( multiplicity ).default_pblock( pb_mid_left ) : kernel::null();
    kernel k_gqa = (GQ_KERNEL_SPLIT == 2) ? sc.def_kernel( "k_gaussian_quadrature_a" ).num( multiplicity ) : kernel::null();
    kernel k_gqb = sc.def_kernel( "k_gaussian_quadrature_b" ).num( multiplicity ).default_slr( 2 );
    kernel k_fb = sc.def_kernel( "k_find_bmax" ).num( multiplicity ).default_pblock( pb_mid_right );
    kernel k_cs = sc.def_kernel( "k_compress_store" ).num( multiplicity );

    if( GQ_KERNEL_SPLIT == 1 && BP_BYPASS == 1 )
    {
        k_prep.default_pblock( pb_bot_left );
        k_rr.default_pblock( pb_mid_left );
        k_cs.default_pblock( pb_bot_right );
    } else if( RR_KERNEL_SPLIT == 2 )
    {
        k_prep.default_pblock( pb_bot_right );
        k_rr_a.default_pblock( pb_bot_left );
        k_rr_b.default_pblock( pb_bot_left );
        k_gqa.default_pblock( pb_mid_left );
        k_cs.default_pblock( pb_mid_right );
    } else if( use_alt )
    {
        if( AM_ABCD < 3300 && AM_ABCD != 3230 && AM_ABCD != 3231)
        {
            sc.add_tcl( "create_pblock pblock_BL_alt" );
            sc.add_tcl( "resize_pblock pblock_BL_alt -add CLOCKREGION_X0Y0:CLOCKREGION_X5Y3 -locs keep_all" );
            k_prep.default_pblock( pb_bot_left_alt );
            for( int m = 0; m < multiplicity; ++m )
            {
                if( m % 2 == 0 )
                    k_rr[m].pblock( pb_bot_left_alt );
                else
                    k_rr[m].pblock( pb_mid_left );
            }
            if( GQ_KERNEL_SPLIT == 2 ) k_gqa.default_slr( 2 );
            k_cs.default_pblock( pb_mid_right );
        } else
        {
            sc.add_tcl( "create_pblock pblock_BL_alt" );
            sc.add_tcl( "resize_pblock pblock_BL_alt -add CLOCKREGION_X0Y0:CLOCKREGION_X5Y3 -locs keep_all" );
            k_prep.default_pblock( pb_bot_left_alt );
            k_rr.default_pblock( pb_bot_left_alt );
            if( GQ_KERNEL_SPLIT == 2 ) k_gqa.default_pblock( pb_mid_left );
            k_cs.default_pblock( pb_mid_right );
        }
    } else if (AM_ABCD == 3311)
    {
        k_prep.default_pblock( pb_bot_right );
        k_rr.default_pblock( pb_bot_left );
        k_gqa.default_pblock( pb_mid_left );
        k_cs.default_pblock( pb_mid_right );
    } else if (AM_ABCD == 3310)
    {
        k_prep.default_pblock( pb_bot_right );
        k_rr.default_pblock( pb_bot_left );
        k_gqa.default_pblock( pb_mid_left );
        k_cs.default_pblock( pb_bot_right );
    } else
    {
        k_prep.default_pblock( pb_bot_left );
        k_rr.default_pblock( pb_bot_left );
        k_gqa.default_pblock( pb_mid_left );
        k_cs.default_pblock( pb_bot_right );
    }

    bundle prep2rr = bundle()
            .add( "xyz_derived_stream" )
            .add( "rys_wt_stream" )
            .add( "tb_b_stream" )
            .add( "tb_c_stream" )
            .add( "n_stream" );

    bundle rr2bp = bundle()
            .add( "ucba_stream"_.count( BP_BYPASS == 0 ? 2 : 4 ) )
            .add( "n_stream" );

    bundle bp2gqa = bundle()
            .add( "uba_stream"_.count( BP_FIFO_COUNT ) )
            .add( "n_stream" );

    bundle gqa2gqb = bundle()
            .add_if( BP_BYPASS == 0, "uba_stream"_.count( BP_FIFO_COUNT ) )
            .add_if( BP_BYPASS == 1, "ucba_stream"_.count( 4 ) )
            .add( "n_stream" );

    bundle gqa2fb = bundle()
            .add( "partial_eris_stream_out"_to( "partial_eris_a_stream_in" ).count( 2 ).fifo( AGGRESSIVE_UNROLL ? 512 : 16 ) ); // when pipelined with aggressive unroll, there is a larger latency between steps

    bundle gqb2fb = bundle()
            .add( "partial_eris_stream_out"_to( "partial_eris_b_stream_in" ).count( 2 ) )
            .add( "n_stream" );

    bundle fb2cs = bundle()
            .add( "cart_eris_stream"_.count( 2 ).fifo( 512 ) )
            .add( "b_max_stream"_.fifo( 16 ) );

    for( int i = 0; i < multiplicity; ++i )
    {
        k_prep[i] << "m_axi_abcd_inp" << hbm( i * 2 + (RR_KERNEL_SPLIT == 2 ? 17 : 0) + 0 ).saxi( i * 2 + (RR_KERNEL_SPLIT == 2 ? 17 : 0) + 0 );
        k_prep[i] << "m_axi_rtwt_rys_inp" << hbm( i * 2 + (RR_KERNEL_SPLIT == 2 ? 17 : 0) + 1 ).saxi( i * 2 + (RR_KERNEL_SPLIT == 2 ? 17 : 0) + 1 );

        if( RR_KERNEL_SPLIT == 1 )
            k_prep[i] >> prep2rr >> k_rr[i];
        else
            k_prep[i] >> prep2rr >> k_rr_a[i] >> prep2rr >> k_rr_b[i];

        if( BP_BYPASS == 0 )
        {
            if( RR_KERNEL_SPLIT == 1)
                k_rr[i] >> rr2bp >> k_bp[i];
            else
            {
                k_rr_a[i] >> "ucba_stream_out_0"_to("ucba_stream_in_0") >> k_bp[i];
                k_rr_b[i] >> "ucba_stream_out_1"_to("ucba_stream_in_1") >> k_bp[i];
                k_rr_b[i] >> "n_stream" >> k_bp[i];
            }
            if( GQ_KERNEL_SPLIT == 2 )
                k_bp[i] >> bp2gqa >> k_gqa[i] >> gqa2gqb >> k_gqb[i];
            else // GQ_KERNEL_SPLIT == 1
                k_bp[i] >> bp2gqa >> k_gqb[i];
        } else // BP_BYPASS == 1
        {
            if( GQ_KERNEL_SPLIT == 2 && RR_KERNEL_SPLIT == 2 )
                k_rr_b[i] >> rr2bp >> k_gqa[i] >> gqa2gqb >> k_gqb[i];
            else if( GQ_KERNEL_SPLIT == 2 )
                k_rr[i] >> rr2bp >> k_gqa[i] >> gqa2gqb >> k_gqb[i];
            else // GQ_KERNEL_SPLIT == 1
                k_rr[i] >> rr2bp >> k_gqb[i];
        }

        if( GQ_KERNEL_SPLIT == 2 ) k_gqa[i] >> gqa2fb >> k_fb[i];

        k_gqb[i] >> gqb2fb >> k_fb[i];
        k_fb[i] >> fb2cs >> k_cs[i];

        for( int o = 0; o < NUM_ERIS_PORTS; ++o )
        {
            constexpr int first_port = use_alt && AM_ABCD >= 3200 ? 21 : (AM_ABCD <= 2020 || (AM_ABCD <= 3030 && AM_ABCD >= 3000) ? 20 : 17);
            int port = first_port + (i * NUM_ERIS_PORTS) + o;
            k_cs[i] << ("m_axi_ce" + to_string( o )) << hbm( port ).saxi( port );
        }
    }

    sc.finalize();
}