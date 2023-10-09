#include "rys_testbench.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include "timer.h"
#include "qc_host_util.h"
#include "bin_file.h"
#include "rys/RysRoots.h"
#include "data_comparator.hpp"


/**
 * sqrt approximation that allows sqrt to be computed at compile-time in c++14
 * c++20 supports the regular sqrt function being used in
 */
fp_t constexpr sqrtNewtonRaphson( fp_t x, fp_t curr, fp_t prev )
{
    return curr == prev
           ? curr
           : sqrtNewtonRaphson( x, (fp_t) 0.5 * (curr + x / curr), curr );
}

/**
* Constexpr version of the square root
* Return value:
*   - For a finite and non-negative value of "x", returns an approximation for the square root of "x"
*   - Otherwise, returns NaN
*/
fp_t constexpr ce_sqrt( fp_t x )
{
    return x >= 0 && x < std::numeric_limits<fp_t>::infinity()
           ? sqrtNewtonRaphson( x, x, 0 )
           : std::numeric_limits<fp_t>::quiet_NaN();
}

/**
 * compute 1 / √ ((2 * l - 1)!!)
 * NOTE: l ∈ [0, 3] and if l is zero, then 2 * l - 1 is -1.
 * Thus, unsigned int cannot be used here.
 *
 * this can be evaluated at compile-time from the angular momentum numbers
 */
constexpr fp_t prefactor( const short int l )
{
    const short int n = 2 * l - 1;
    fp_t t { 1.0 };
    // double factorial
    for( short int i { n }; i > 1; i -= 2 )
        t *= i;
    return (fp_t) 1.0 / ce_sqrt( t );
}

bool qcf::rys_testbench::run( const qcf::mob_file& mob, qcf::rys_vitis_kernel& rys_quadrature_kernel, const int am_abcd[four] )
{
    // check info
    q_info = mob.calc_info( am_abcd );
    std::cout << "Running test-bench for [" << mob.get_filepath() << "] "
              << "target ERI quartet: " << qcf::util::quartet_friendly_name( am_abcd ) << std::endl
              << " number of Cart-ERIs per quartet: " << q_info.num_cart_eris_per_quartet << std::endl
              << " number of quartets  in molecule: " << q_info.num_quartets << std::endl
              << " number of Cart-ERIs in molecule: " << q_info.num_cart_eris
              << " ≈ " << 1.0e-6 * (double) q_info.num_cart_eris << " MERIs" << std::endl
              << std::endl
              << " memory size for all ERIs (in MB): "
              << (double) (sizeof( double ) * q_info.num_cart_eris) * 1.0e-6 << std::endl
              << std::endl;

    // generate input data
    std::cout << "Calculating input data" << std::endl;
    vector<tuple<qp_t, rys_t>> input_data_vec = generate_input_data( mob, am_abcd );

    // run calculations with kernel
    all_eris_mol = vector<array<double, num_cart_eris_quartet>>();
    std::cout << "Executing kernel" << std::endl;
    if( !rys_quadrature_kernel.get_benchmark_mode() )
    {
        std::cout << "\033[33mWarning: Benchmark mode is not enabled (-b). Collected result metrics may not be accurate.\033[0m" << std::endl;
        std::cout << " - Benchmark mode is recommended to be disabled during sw_emu & hw_emu. And should be enabled for hw run." << std::endl;
    }
    qcf::timer host_timer;

    // collect data
    std::vector<qp_t> qp_data_vec;
    std::vector<rys_t> rys_data_vec;
    for( const auto& input_data: input_data_vec )
    {
        qp_data_vec.push_back( std::get<0>( input_data ) );
        rys_data_vec.push_back( std::get<1>( input_data ) );
    }

    double* all_cart_eris;
    std::vector<double> execution_times; // To store the execution time of each run
    const int num_runs = rys_quadrature_kernel.get_power_measure_duration_seconds() > 0 || !rys_quadrature_kernel.get_benchmark_mode() ? 1 : 5; // Number of times the kernel is invoked

    for( int i = 0; i < num_runs; ++i )
    {
        // Invoke kernel with collected data
        all_cart_eris = rys_quadrature_kernel.invoke( qp_data_vec.data(), rys_data_vec.data(), qp_data_vec.size() );

        // Store the execution time for the current run
        execution_times.push_back( rys_quadrature_kernel.get_last_kernel_exec_time_ms() );
    }

    // re-order results and accumulate
    for( size_t i = 0; i < qp_data_vec.size(); ++i )
    {
        double* cart_eris = all_cart_eris + i * num_cart_eris_quartet;

        array<double, num_cart_eris_quartet> cart_eris_for_verification = reorder_cart_eris_for_verification( cart_eris );
        all_eris_mol.push_back( cart_eris_for_verification );
    }

    double average_time_ms = std::accumulate( execution_times.begin(), execution_times.end(), 0.0 ) / execution_times.size();
    double fastest_time_ms = *std::min_element( execution_times.begin(), execution_times.end() );

    double elapsed_time_ms = host_timer.elapsed_ms();

    if( num_runs > 1 )
    {
        std::ofstream file( "bm_results_" + std::to_string(am_abcd[0]) + std::to_string(am_abcd[1]) + std::to_string(am_abcd[2]) + std::to_string(am_abcd[3]) + ".json" ); // Opens a file named "output.json" for writing
        // write out json
        file << "{\n";
        file << "\"detected_multiplicity\": " << rys_quadrature_kernel.get_multiplicity() << ",\n";
        file << "\"verification_passed\": " << (false ? "true" : "false") << ",\n";
        file << "\"measured_throughput_max\": " << (1.0e-3 * (double) (q_info.num_cart_eris / fastest_time_ms)) << ",\n";
        file << "\"measured_throughput_avg\": " << (1.0e-3 * (double) (q_info.num_cart_eris / average_time_ms)) << ",\n";
        file << "\"pc_capacity_util\": " << rys_quadrature_kernel.get_max_pc_util() << ",\n";
        file << "\"num_runs\": " << num_runs << "\n";
        file << "}\n";
        file.close();
    }

    // write results to output file
    string out_filename = "calc_" + qcf::util::quartet_simple_name( am_abcd ) + ".bin";
    qcf::bin_file::write_file( out_filename, all_eris_mol.data(), sizeof( double ) * all_eris_mol.size() * num_cart_eris_quartet );
    std::cout << "Wrote results to: " << out_filename << std::endl;

    if( average_time_ms != 0 )
        std::cout << "Kernel exec: " << 1.0e-3 * (average_time_ms) << " sec -> " << 1.0e-3 * (double) (q_info.num_cart_eris / average_time_ms) << " MERIs/s (MERIS)   " << qcf::util::quartet_simple_name( am_abcd ) << std::endl;
    else
        std::cout << "Error reading kernel time" << std::endl;
    std::cout << "Net calc exec: " << 1.0e-3 * (elapsed_time_ms) << " sec -> " << 1.0e-3 * (double) (q_info.num_cart_eris / elapsed_time_ms) << " MERIs/s (MERIS)" << std::endl;
    return true;
}

bool rys_testbench::verify_correctness( string reference_filepath )
{
    size_t actual_num_eris = all_eris_mol.size() * num_cart_eris_quartet;
    if( actual_num_eris != q_info.num_cart_eris )
    {
        std::cout << "Produced eris contains only " << actual_num_eris << " values which doesn't match the expected " << q_info.num_cart_eris << " values" << std::endl;
        return false;
    }

    size_t expected_size_bytes;
    double* expected_data = (double*) qcf::bin_file::read_file( reference_filepath, expected_size_bytes );
    if( expected_size_bytes != sizeof( double ) * q_info.num_cart_eris )
    {
        std::cout << "Reference file size " << expected_size_bytes << " B doesn't match expected " << (sizeof( double ) * q_info.num_cart_eris) << " B" << std::endl;
        return false;
    }

    qcf::data_comparator<double>::elem_info max_info = qcf::data_comparator<double>::find_max_diff( expected_data, (double*) all_eris_mol.data(), q_info.num_cart_eris );
    std::cout << "Maximum error is " << max_info.diff << ": index = " << max_info.index << " expected = " << max_info.a << " actual = " << max_info.b << std::endl;
    vector<qcf::data_comparator<double>::elem_info> threshold_infos = qcf::data_comparator<double>::find_above_threshold( expected_data, (double*) all_eris_mol.data(), q_info.num_cart_eris, eri_threshold );
    std::cout << "There are " << threshold_infos.size() << " indices above the error threshold of " << eri_threshold << std::endl;
    size_t print_indices_count = std::min( threshold_infos.size(), max_print_indices );
    if( print_indices_count > 0 )
    {
        std::cout << "Those indices are:";
        for( int i = 0; i < print_indices_count; ++i )
        {
            if( i != 0 ) std::cout << ",";
            std::cout << " " << threshold_infos[i].index;
        }
        if( threshold_infos.size() > print_indices_count )
            std::cout << " (and " << (threshold_infos.size() - print_indices_count) << " more)";
        std::cout << std::endl;

        for( int i = 0; i < print_indices_count; ++i )
            std::cout << "idx(" << threshold_infos[i].index << ") actual: " << threshold_infos[i].b << " != " << threshold_infos[i].a << std::endl;
    }
    return threshold_infos.empty();
}

array<double, num_cart_eris_quartet> qcf::rys_testbench::reorder_cart_eris_for_verification( const double* cart_eris )
{
    std::array<double, num_cart_eris_quartet> cart_eris_for_verification;
    for( int igd = 0; igd < ncg_d; igd++ )
    {
        for( int igc = 0; igc < ncg_c; igc++ )
        {
            for( int igb = 0; igb < ncg_b; igb++ )
            {
                for( int iga = 0; iga < ncg_a; iga++ )
                {
                    cart_eris_for_verification[iga * ncg_b * ncg_c * ncg_d +
                                               igb * ncg_c * ncg_d +
                                               igc * ncg_d +
                                               igd] =
                            cart_eris[igd * ncg_c * ncg_b * ncg_a +
                                      igc * ncg_b * ncg_a +
                                      igb * ncg_a +
                                      iga];
                }
            }
        }
    }
    return cart_eris_for_verification;
}

vector<tuple<qp_t, rys_t>> qcf::rys_testbench::generate_input_data( const qcf::mob_file& mob, const int am_abcd[four] )
{
    vector<tuple<qp_t, rys_t>> input_data_vec;

    const vector<double>& xyz = mob.get_xyz();
    const vector<double>& xa = mob.get_xa();

    qcf::mob_quartet_info_t q_info = mob.calc_info( am_abcd );

    // below this point is still a mess

    /*****************************************************************************
     *
     * input and output of the kernel
     *
     ****************************************************************************/
    std::array<double, four * nxyz> xyz_abcd_inp {};
    std::array<double, four> exp_abcd_inp {};
    std::array<double, ord_rys * 2> rtwt_rys_inp {};


    qcf::timer host_timer;
    for( auto a { 0U }; a < q_info.list_abcd[0].size(); a++ )
    { // [a|
        auto ibf_a = q_info.list_abcd[0].at( a ); // index of the basis function for a
        exp_abcd_inp[0] = xa[ibf_a];     // α
        for( int i { 0 }; i < nxyz; i++ ) // A
            xyz_abcd_inp[0 + i] = xyz[nxyz * ibf_a + i];
        for( auto b { 0U }; b < q_info.list_abcd[1].size(); b++ )
        { // [b|
            auto ibf_b = q_info.list_abcd[1].at( b ); // index of the basis function for b
            exp_abcd_inp[1] = xa[ibf_b];     // β
            for( int i { 0 }; i < nxyz; i++ ) // B
                xyz_abcd_inp[3 + i] = xyz[nxyz * ibf_b + i];
            double zt { exp_abcd_inp[0] + exp_abcd_inp[1] }; // ζ = α + β
            std::array<double, nxyz> p;
            for( int i { 0 }; i < nxyz; i++ )
            {               // P = (α A + β B) / ζ
                p[i] = (exp_abcd_inp[0] * xyz_abcd_inp[0 + i]
                        + exp_abcd_inp[1] * xyz_abcd_inp[3 + i]) / zt;
            }
            for( auto c { 0U }; c < q_info.list_abcd[2].size(); c++ )
            { // |c]
                auto ibf_c = q_info.list_abcd[2].at( c ); // index of the basis function for c
                exp_abcd_inp[2] = xa[ibf_c];     // γ
                for( int i { 0 }; i < nxyz; i++ ) // C
                    xyz_abcd_inp[6 + i] = xyz[nxyz * ibf_c + i];
                for( auto d { 0U }; d < q_info.list_abcd[3].size(); d++ )
                { // |d]
                    auto ibf_d = q_info.list_abcd[3].at( d ); // index of the basis function for d
                    exp_abcd_inp[3] = xa[ibf_d];     // δ
                    for( int i { 0 }; i < nxyz; i++ ) // D
                        xyz_abcd_inp[9 + i] = xyz[nxyz * ibf_d + i];
                    double et { exp_abcd_inp[2] + exp_abcd_inp[3] }; // η = γ + δ
                    double ab2 = 0.0;
                    double cd2 = 0.0;
                    std::array<double, nxyz> q;
                    double rhopq2 { 0.0 }; // ρ * |PQ|²
                    for( int i { 0 }; i < nxyz; i++ )
                    {               // Q = (γ C + δ D) / η
                        q[i] = (exp_abcd_inp[2] * xyz_abcd_inp[6 + i]
                                + exp_abcd_inp[3] * xyz_abcd_inp[9 + i]) / et;
                        rhopq2 += (p[i] - q[i]) * (p[i] - q[i]); // |PQ|²
                        ab2 += std::pow( xyz_abcd_inp[0 * nxyz + i] - xyz_abcd_inp[1 * nxyz + i], 2 );        // |AB|²
                        cd2 += std::pow( xyz_abcd_inp[2 * nxyz + i] - xyz_abcd_inp[3 * nxyz + i], 2 );        // |CD|²
                    }
                    rhopq2 *= zt * et / (zt + et); // ρ * |PQ|², where ρ = ζ * η / (ζ + η)
                    /*
                     * compute the roots and weights of the Rys polynomials
                     *
                     * NOTE:
                     * - computational complexity is O(l), l is the sum of angular momentum
                     * - use CINTrys_roots from libcint, which copies from GAMESS
                     *
                     * libcint: (u_μ)² and w_μ are computed
                     */
                    std::array<double, ord_rys> roots;
                    std::array<double, ord_rys> wghts;
                    CINTrys_roots( ord_rys, rhopq2, roots.data(), wghts.data() );
                    for( int i { 0 }; i < ord_rys; i++ )
                    {
                        rtwt_rys_inp[0 + i] = roots[i];
                        rtwt_rys_inp[ord_rys + i] = wghts[i];
                    }

                    fp_t nc4 { 1.0 }; // Nc_a * Nc_b * Nc_c * Nc_d
                    for( int_t k = 0; k < four; k++ )
                    {
                        double t = two_over_pi * exp_abcd_inp[k];
                        nc4 *= std::sqrt( std::sqrt( t * t * t )
                                          * std::pow( 4.0 * exp_abcd_inp[k], am_abcd[k] ) )
                               * prefactor( am_abcd[k] );
                    }

                    double zt1 = 1.0 / zt;                  // 1 / ζ
                    double et1 = 1.0 / et;                  // 1 / η
                    double kc = two_pi_52 * nc4 * std::exp( -exp_abcd_inp[0] * exp_abcd_inp[1] * ab2 * zt1 - exp_abcd_inp[2] * exp_abcd_inp[3] * cd2 * et1 ) / (zt * et * std::sqrt( zt + et ));

                    for( int i { 0 }; i < ord_rys; i++ )
                    {
                        // transform (u_μ)² to (t_μ)²
                        rtwt_rys_inp[0 + i] = rtwt_rys_inp[i] / (1.0 + rtwt_rys_inp[i]);
                        // modified weights of  w_μ
                        rtwt_rys_inp[ord_rys + i] = std::cbrt( rtwt_rys_inp[ord_rys + i] * kc );
                    }

                    qp_t abcd_inp = {};
                    rys_t rys_inp = {};
                    // Populate qp_t struct
                    for( int i = 0; i < four; ++i )
                    {
                        for( int j = 0; j < nxyz; ++j )
                        {
                            abcd_inp.xyz_abcd[i][j] = (fp_t) xyz_abcd_inp[i * nxyz + j];
                        }
                        abcd_inp.exp_abcd[i] = (fp_t) exp_abcd_inp[i];
                    }

                    // Populate rys_t struct
                    for( int i = 0; i < ord_rys; ++i )
                    {
                        rys_inp.rt[i] = (fp_t) rtwt_rys_inp[i];
                        rys_inp.wt[i] = (fp_t) rtwt_rys_inp[ord_rys + i];
                    }

                    input_data_vec.emplace_back( abcd_inp, rys_inp );
                } // |d]
            } // |c]
        } // [b|
    } // [a|

    return input_data_vec;
}
