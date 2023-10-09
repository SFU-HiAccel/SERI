#include <iostream>
#include <string>
#include "popl.hpp"
#include "mob_file.h"
#include "rys_vitis_kernel.h"
#include "rys_testbench.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using namespace popl;

void parse_args( int argc, char* const* argv, string& mob_filepath, string& xclbin_filepath, string& ref_eris_filepath, bool& benchmark_mode, float& power_measure_invoke_repeat, bool& wait_before_kernel_exec );
void block_until_keypress();

int main( int argc, char** argv )
{
    string mob_filepath;
    string xclbin_filepath;
    string ref_eris_filepath;
    bool benchmark_mode;
    float power_measure_duration_seconds;
    bool wait_before_kernel_exec;

    parse_args( argc, argv, mob_filepath, xclbin_filepath, ref_eris_filepath, benchmark_mode, power_measure_duration_seconds, wait_before_kernel_exec );

    qcf::mob_file mob_data( mob_filepath );
    qcf::rys_vitis_kernel rys_quadrature_kernel( xclbin_filepath, (int) mob_data.calc_info( am_abcd ).num_quartets );
    rys_quadrature_kernel.set_power_measure_duration_seconds( power_measure_duration_seconds );
    rys_quadrature_kernel.set_benchmark_mode( benchmark_mode );

    if( wait_before_kernel_exec ) block_until_keypress();

    qcf::rys_testbench testbench;
    testbench.run( mob_data, rys_quadrature_kernel, am_abcd );

    if( !ref_eris_filepath.empty() )
    {
        bool passed = testbench.verify_correctness( ref_eris_filepath );
        cout << "Verification " << (passed ? "passed." : "FAILED!") << endl;
    }

    return EXIT_SUCCESS;
}

void parse_args( int argc, char* const* argv, string& mob_filepath, string& xclbin_filepath, string& ref_eris_filepath, bool& benchmark_mode, float& power_measure_invoke_repeat, bool& wait_before_kernel_exec )
{
    OptionParser op( "Options" );
    auto help_option = op.add<Switch>( "h", "help", "produce help message" );
    auto mob_filepath_option = op.add<Value<string>>( "m", "mob", "mob file path", "", &mob_filepath );
    auto xclbin_filepath_option = op.add<Value<string>>( "x", "xclbin", "xclbin file path", "", &xclbin_filepath );
    auto ref_eris_filepath_option = op.add<Value<string>>( "r", "ref", "reference eris file path", "", &ref_eris_filepath );
    auto benchmark_flag = op.add<Switch>( "b", "benchmark", "enable for throughput benchmarking. will invoke the kernel 5 times and report average measurements as well as run the input data multiple times to ensure runtime is long enough for stable results (especially useful for smaller quartet classes)");
    auto power_measure_invoke_repeat_option = op.add<Value<float>>( "d", "duration", "continuously invoke the kernel with same data for duration d for power measurement purposes", 0, &power_measure_invoke_repeat );
    auto wait_before_exec_flag = op.add<Switch>( "w", "wait", "wait for keypress after fpga initialized, but before kernel is executed. for chipscope debugging" );

    op.parse( argc, argv );

    bool show_help = false;
    if( help_option->is_set() )
    {
        cout << op << endl;
        exit( EXIT_SUCCESS );
    }
    if( !mob_filepath_option->is_set() )
    {
        cerr << "Missing --" << mob_filepath_option->long_name() << " option" << endl;
        show_help = true;
    }
    if( !xclbin_filepath_option->is_set() )
    {
        cerr << "Missing --" << xclbin_filepath_option->long_name() << " option" << endl;
        show_help = true;
    }
    wait_before_kernel_exec = wait_before_exec_flag->is_set();
    benchmark_mode = benchmark_flag->is_set();

    if( show_help )
    {
        cout << op << endl;
        exit( EXIT_FAILURE );
    }
}

void block_until_keypress()
{
    cout << "Device initialized, press Enter to continue with kernel execution . . . ";
    std::cin.get();
}
