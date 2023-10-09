#ifndef QC_FPGA_RYS_TESTBENCH_H
#define QC_FPGA_RYS_TESTBENCH_H

#include <vector>
#include <tuple>
#include <array>
#include "mob_file.h"
#include "rys_vitis_kernel.h"
#include "common/types.h"

using std::vector;
using std::array;
using std::tuple;

namespace qcf
{
    static const double eri_threshold = 2.1e-5;
    static const size_t max_print_indices = 512;

    class rys_testbench
    {
    public:
        bool run( const mob_file& mob, rys_vitis_kernel& rys_quadrature_kernel, const int am_abcd[four] );
        bool verify_correctness( string expected_filepath );
    private:
        mob_quartet_info_t q_info;
        vector<array<double, num_cart_eris_quartet>> all_eris_mol;

        static vector<tuple<qp_t, rys_t>> generate_input_data( const mob_file& mob, const int am_abcd[four] );
        static array<double, num_cart_eris_quartet> reorder_cart_eris_for_verification( const double* cart_eris );
    };
}


#endif //QC_FPGA_RYS_TESTBENCH_H
