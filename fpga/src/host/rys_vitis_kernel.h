#ifndef QC_FPGA_RYS_VITIS_KERNEL_H
#define QC_FPGA_RYS_VITIS_KERNEL_H

#include <array>
#include "common/parameters.h"
#include "common/repeat.h"
#include "common/types.h"
#include "vitis_kernel.h"

namespace qcf
{
    class rys_vitis_kernel : public vitis_kernel
    {
    public:
        explicit rys_vitis_kernel( const string& xclbin_filepath, int max_batch_size );
        double* invoke(const qp_t* abcd_data, const rys_t* rys_data, int n);
        double get_last_kernel_exec_time_ms() const;
        int get_multiplicity() const;
        float get_max_pc_util() const;
    private:
        int multiplicity;
        int duplication_capacity;
        float max_pc_util;
        cl::Kernel* k_preparation;
        cl::Kernel* k_compress_store;

        qp_t* abcd_inp;
        rys_t* rys_inp;
#define DEFINE_CART_ERIS( z ) packed_eri_t** cart_eris_##z;
        REPEAT( DEFINE_CART_ERIS, NUM_ERIS_PORTS )
        std::vector<double> cart_eris;

        cl::Buffer* buffer_abcd_inp;
        cl::Buffer* buffer_rys_inp;
#define DEFINE_CL_CART_ERIS_BUF( z ) cl::Buffer* buffer_cart_eris_##z;
        REPEAT( DEFINE_CL_CART_ERIS_BUF, NUM_ERIS_PORTS )
        double last_kernel_exec_time_ms;
    };
}


#endif //QC_FPGA_RYS_VITIS_KERNEL_H
