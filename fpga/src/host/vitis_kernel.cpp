#include "vitis_kernel.h"

qcf::vitis_kernel::vitis_kernel( const string& xclbin_filepath )
{
    auto devices = xcl::get_xil_devices();
    auto fileBuf = xcl::read_binary_file( xclbin_filepath );
    cl::Program::Binaries bins { { fileBuf.data(), fileBuf.size() } };

    bool valid_device = false;
    for( unsigned int i = 0; i < devices.size(); i++ )
    {
        auto device = devices[i];
        cl_int err;

        // create context and command queue for selected device
        OCL_CHECK( err, context = cl::Context( device, nullptr, nullptr, nullptr, &err ) );
        OCL_CHECK( err, cq = cl::CommandQueue( context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE, &err ) );

        // try to program device
        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        program = cl::Program( context, { device }, bins, nullptr, &err );

        // if success, stop searching for device
        if( err == CL_SUCCESS )
        {
            valid_device = true;
            break;
        }

        // if failed, continue loop and try another device
        std::cout << "Error: Failed to program device[" << i << "] with xclbin file!" << std::endl;
    }

    if( !valid_device )
    {
        std::cout << "Failed to program any device found, exit!" << std::endl;
        exit( EXIT_FAILURE );
    }
}


void qcf::vitis_kernel::set_power_measure_duration_seconds( float power_measure_duration_seconds )
{
    this->power_measure_duration_seconds = power_measure_duration_seconds;
}

float qcf::vitis_kernel::get_power_measure_duration_seconds() const
{
    return this->power_measure_duration_seconds;
}

void qcf::vitis_kernel::set_benchmark_mode( bool benchmark_mode )
{
    this->benchmark_mode = benchmark_mode;
}

bool qcf::vitis_kernel::get_benchmark_mode() const
{
   return this->benchmark_mode;
}

