#include "rys_vitis_kernel.h"
#include <chrono>

qcf::rys_vitis_kernel::rys_vitis_kernel( const string& xclbin_filepath, int max_batch_size ) : vitis_kernel( xclbin_filepath )
{
    size_t est_bank_size = sizeof( packed_eri_t ) * ncg_c * ncg_d * max_batch_size;
    if( est_bank_size > HBM_PC_SIZE )
    {
        printf( "Estimated size of output bank %zu exceeds maximum hbm PC size %d\n", est_bank_size, HBM_PC_SIZE );
        exit( 1 );
    }
    duplication_capacity = HBM_PC_SIZE / std::max( std::max( est_bank_size, sizeof( qp_t ) * max_batch_size ), sizeof( rys_t ) * max_batch_size );
    max_pc_util = ((float) est_bank_size * (float) duplication_capacity) / (float) HBM_PC_SIZE;

    cl_int err;
    multiplicity = 0;
    for( int m = 0; m < 20; ++m )
    {
        string k_prep_name = "k_preparation:{k_preparation_" + std::to_string( m + 1 ) + "}";

        cl::Kernel( program, k_prep_name.c_str(), &err );
        if( err != CL_SUCCESS )
        {
            printf( "%s:%d Error calling cl::Kernel( program, k_prep_name.c_str(), &err ), [k_prep_name='%s'], error code is: %d (this error is not necessarily fatal)\n", __FILE__, __LINE__, k_prep_name.c_str(), err );
            break;
        }
        multiplicity = m + 1;

    }
    if( multiplicity == 0 )
    {
        printf( "Failed to find any instances of accelerator (detected multiplicity == 0)\n" );
        exit( 1 );
    }
    std::cout << "Auto-detected " << multiplicity << " kernel instance(s). Using multiplicity " << multiplicity << std::endl;


    abcd_inp = new qp_t[max_batch_size * duplication_capacity];
    rys_inp = new rys_t[max_batch_size * duplication_capacity];
#define ALLOC_CART_ERIS_MULT( z ) cart_eris_##z = new packed_eri_t*[multiplicity];
    REPEAT( ALLOC_CART_ERIS_MULT, NUM_ERIS_PORTS )

    cart_eris.resize( num_cart_eris_quartet * max_batch_size );

    k_preparation = new cl::Kernel[multiplicity];
    k_compress_store = new cl::Kernel[multiplicity];
    buffer_abcd_inp = new cl::Buffer[multiplicity];
    buffer_rys_inp = new cl::Buffer[multiplicity];
#define ALLOC_CART_ERIS_BUFS( z ) buffer_cart_eris_##z = new cl::Buffer[multiplicity];
    REPEAT( ALLOC_CART_ERIS_BUFS, NUM_ERIS_PORTS )


    for( int m = 0; m < multiplicity; ++m )
    {
        // alloc cart eris bufs
//#define ALLOC_CART_ERIS( z ) cart_eris_##z[m] = new packed_eri_t[max_batch_size];
#define ALLOC_CART_ERIS( z ) cart_eris_##z[m] = (packed_eri_t*)malloc(HBM_PC_SIZE);
        REPEAT( ALLOC_CART_ERIS, NUM_ERIS_PORTS )

        // get kernels
        string k_prep_name = "k_preparation:{k_preparation_" + std::to_string( m + 1 ) + "}";
        string k_compress_store_name = "k_compress_store:{k_compress_store_" + std::to_string( m + 1 ) + "}";

        k_preparation[m] = cl::Kernel( program, k_prep_name.c_str(), &err );
        if( err != CL_SUCCESS )
        {
            printf( "%s:%d Error calling cl::Kernel( program, k_prep_name.c_str(), &err ), [k_prep_name='%s'], error code is: %d (this error is not necessarily fatal)\n", __FILE__, __LINE__, k_prep_name.c_str(), err );
            break;
        }
        OCL_CHECK( err, k_compress_store[m] = cl::Kernel( program, k_compress_store_name.c_str(), &err ) );

        // allocate buffer memory
        OCL_CHECK( err, buffer_abcd_inp[m] = cl::Buffer(
                context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                sizeof( qp_t ) * max_batch_size * duplication_capacity, abcd_inp, &err
        ) );
        OCL_CHECK( err, buffer_rys_inp[m] = cl::Buffer(
                context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                sizeof( rys_t ) * max_batch_size * duplication_capacity, rys_inp, &err
        ) );
#define ALLOC_CL_CART_ERIS_BUF( z ) OCL_CHECK( err, buffer_cart_eris_##z[m] = cl::Buffer( \
            context, CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, \
                    (size_t)HBM_PC_SIZE, cart_eris_##z[m], &err \
            ) );
        REPEAT( ALLOC_CL_CART_ERIS_BUF, NUM_ERIS_PORTS )

#define SET_ARG_CL_CART_ERIS_BUF( z ) OCL_CHECK( err, err = k_compress_store[m].setArg( z+3, buffer_cart_eris_##z[m] ) );
        OCL_CHECK( err, err = k_preparation[m].setArg( 0, buffer_abcd_inp[m] ) );
        OCL_CHECK( err, err = k_preparation[m].setArg( 1, buffer_rys_inp[m] ) );
        REPEAT( SET_ARG_CL_CART_ERIS_BUF, NUM_ERIS_PORTS )

    }

}

double* qcf::rys_vitis_kernel::invoke( const qp_t* abcd_data, const rys_t* rys_data, int n )
{
    int effective_n = get_benchmark_mode() ? std::min( n * duplication_capacity, (1 << 21) - 1) : n;

    memcpy( abcd_inp, abcd_data, sizeof( qp_t ) * n );
    memcpy( rys_inp, rys_data, sizeof( rys_t ) * n );

    // copy data from host to device
    cl_int err;
    cl::Event* event_preparation, * event_compress_store;
    event_preparation = new cl::Event[multiplicity];
    event_compress_store = new cl::Event[multiplicity];

    for( int m = 0; m < multiplicity; ++m )
    {
        OCL_CHECK( err, err = k_preparation[m].setArg( 2, effective_n ) );
        OCL_CHECK( err, err = k_compress_store[m].setArg( NUM_ERIS_PORTS + 3, effective_n ) );
        OCL_CHECK( err, err = cq.enqueueMigrateMemObjects( { buffer_abcd_inp[m], buffer_rys_inp[m] }, 0 /* 0 means from host*/) );
    }
    cq.finish();

    if( power_measure_duration_seconds > 0 )
    {
        std::cout << "Continuously invoking kernel for " << power_measure_duration_seconds << " seconds for power consumption measurement (timing/throughput results may be inaccurate)." << std::endl;

        auto start = std::chrono::steady_clock::now();

        // estimate the number of calls needed to run for that amount time given perfomrance model
        int burst_num = 240000 * (unsigned long) (power_measure_duration_seconds * 1000) / (effective_n * ncg_d * ncg_c);
        std::cout << "Calculated a bust of " << burst_num << " invocations." << std::endl;

        for( int b = 0; b < burst_num; ++b )
        {
            for( int m = 0; m < multiplicity; ++m )
            {
                OCL_CHECK( err, err = cq.enqueueTask( k_preparation[m], nullptr, &event_preparation[m] ) );
            }
            for( int m = 0; m < multiplicity; ++m )
            {
                OCL_CHECK( err, err = cq.enqueueTask( k_compress_store[m], nullptr, &event_compress_store[m] ) );
            }
        }
        cq.finish();
        auto end = std::chrono::steady_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Invocations finished. (ran for " << duration << " ms)" << std::endl;
    } else
    {
        for( int m = 0; m < multiplicity; ++m )
        {
            OCL_CHECK( err, err = cq.enqueueTask( k_preparation[m], nullptr, &event_preparation[m] ) );
        }
        for( int m = 0; m < multiplicity; ++m )
        {
            OCL_CHECK( err, err = cq.enqueueTask( k_compress_store[m], nullptr, &event_compress_store[m] ) );
        }
        cq.finish();
    }

    // copy results from device to host
#define PASS_CL_CART_ERIS_BUF( z ) buffer_cart_eris_##z[0],
    OCL_CHECK( err, err = cq.enqueueMigrateMemObjects(
            {
                    REPEAT( PASS_CL_CART_ERIS_BUF, NUM_ERIS_PORTS )
            }, CL_MIGRATE_MEM_OBJECT_HOST
    ) );
    cq.finish();

    // Measure execution time
    cl_ulong net_start_time = CL_ULONG_MAX;
    cl_ulong net_end_time = 0;
    for( int m = 0; m < multiplicity; ++m )
    {
        cl_ulong start_time, end_time;
        event_preparation[m].getProfilingInfo( CL_PROFILING_COMMAND_START, &start_time );
        event_compress_store[m].getProfilingInfo( CL_PROFILING_COMMAND_END, &end_time );
        net_start_time = std::min( net_start_time, start_time );
        net_end_time = std::max( net_end_time, end_time );
        cl_ulong execution_time_ns = end_time - start_time;
        std::cout << "Kernel Inst " << m << " Start: " << start_time << " ns" << std::endl;
        std::cout << "Kernel Inst " << m << " End  : " << end_time << " ns" << std::endl;
        std::cout << "Kernel Inst " << m << " Exec : " << execution_time_ns << " ns" << std::endl;
    }

    cl_ulong net_execution_time_ns = net_end_time - net_start_time;
    std::cout << "Net: " << net_execution_time_ns << " ns" << std::endl;
    last_kernel_exec_time_ms = net_execution_time_ns * 1e-6 / multiplicity / ((float)effective_n / (float)n); // since we compuled m times number of data, adjust

    memset( cart_eris.data(), 0xff, cart_eris.size() * sizeof( double ) );

    int idx = 0;
    // combine data from multiple arrays in single array
    for( int b = 0; b < n; ++b )
    {
        fp_t epsilon;
        // duplicate
        constexpr int past_last_eri_index = (ncg_b * ncg_a) - ((num_eris_ports - 1) * pack_count);
#define LAST_CART_ERIS_GET_E( z ) memcpy( &epsilon, &cart_eris_##z[0][b * ncg_d * ncg_c].data[past_last_eri_index], sizeof( fp_t ) );
        FOR_LAST( LAST_CART_ERIS_GET_E, NUM_ERIS_PORTS )

        for( int i = 0; i < ncg_d * ncg_c; i++ )
        {
            int p_idx = 0;
#define TRANSFER_CART_ERIS( z ) for( int j = 0; j < pack_count; j++) if(p_idx++ < ncg_b * ncg_a) cart_eris[idx++] = static_cast<double>(cart_eris_##z[0][b * ncg_d * ncg_c + i].data[j]) * epsilon;
            REPEAT( TRANSFER_CART_ERIS, NUM_ERIS_PORTS )

        }
    }

    return cart_eris.data();
}

double rys_vitis_kernel::get_last_kernel_exec_time_ms() const
{
    return last_kernel_exec_time_ms;
}

int rys_vitis_kernel::get_multiplicity() const
{
    return multiplicity;
}

float rys_vitis_kernel::get_max_pc_util() const
{
    return max_pc_util;
}
