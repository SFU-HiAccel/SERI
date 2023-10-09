#ifndef QC_FPGA_VITIS_KERNEL_H
#define QC_FPGA_VITIS_KERNEL_H

#include <string>
#include "xcl2/xcl2.hpp"

#define HBM_PC_SIZE ( 1 << 28 )

using std::string;

namespace qcf
{
    class vitis_kernel
    {
    public:
        /**
         * Continuously invoke the kernel a number of times. Useful for measuring power consumption.
         * @param powerBmInvokeRepeat
         */
        void set_power_measure_duration_seconds( float power_measure_duration_seconds );
        void set_benchmark_mode( bool benchmark_mode );
        float get_power_measure_duration_seconds() const;
        bool get_benchmark_mode() const;
    protected:
        explicit vitis_kernel( const string& xclbin_filepath );

        cl::CommandQueue cq;
        cl::Context context;
        cl::Program program;
        float power_measure_duration_seconds = 0;
        bool benchmark_mode = false;
    };
}


#endif //QC_FPGA_VITIS_KERNEL_H
