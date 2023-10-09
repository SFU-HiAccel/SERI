#ifndef QC_FPGA_PARAM_CONFIG_H
#define QC_FPGA_PARAM_CONFIG_H

#define RR_KERNEL_SPLIT 1

#if AM_ABCD == 0000
#define ORD_RYS 1
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 1000
#define ORD_RYS 1
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 1010
#define ORD_RYS 2
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 1100
#define ORD_RYS 2
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 1110
#define ORD_RYS 2
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 1111
#define ORD_RYS 3
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2000
#define ORD_RYS 2
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2010
#define ORD_RYS 2
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2011
#define ORD_RYS 3
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2020
#define ORD_RYS 3
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2100
#define ORD_RYS 2
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2110
#define ORD_RYS 3
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2111
#define ORD_RYS 3
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2120
#define ORD_RYS 3
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2121
#define ORD_RYS 4
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2200
#define ORD_RYS 3
#define NUM_ERIS_PORTS 3
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2210
#define ORD_RYS 3
#define NUM_ERIS_PORTS 3
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2211
#define ORD_RYS 4
#define NUM_ERIS_PORTS 3
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2220
#define ORD_RYS 4
#define NUM_ERIS_PORTS 3
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2221
#define ORD_RYS 4
#define NUM_ERIS_PORTS 3
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 2222
#define ORD_RYS 5
#define NUM_ERIS_PORTS 3
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3000
#define ORD_RYS 2
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3010
#define ORD_RYS 3
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3011
#define ORD_RYS 3
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3020
#define ORD_RYS 3
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3021
#define ORD_RYS 4
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3022
#define ORD_RYS 4
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3030
#define ORD_RYS 4
#define NUM_ERIS_PORTS 1
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 1
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3100
#define ORD_RYS 3
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3110
#define ORD_RYS 3
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3111
#define ORD_RYS 4
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3120
#define ORD_RYS 4
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3121
#define ORD_RYS 4
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3122
#define ORD_RYS 5
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3130
#define ORD_RYS 4
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3131
#define ORD_RYS 5
#define NUM_ERIS_PORTS 2
#define BP_FIFO_COUNT 6
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3200
#define ORD_RYS 3
#define NUM_ERIS_PORTS 4
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3210
#define ORD_RYS 4
#define NUM_ERIS_PORTS 4
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3211
#define ORD_RYS 4
#define NUM_ERIS_PORTS 4
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3220
#define ORD_RYS 4
#define NUM_ERIS_PORTS 4
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3221
#define ORD_RYS 5
#define NUM_ERIS_PORTS 4
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3222
#define ORD_RYS 5
#define NUM_ERIS_PORTS 4
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3230
#define ORD_RYS 5
#define NUM_ERIS_PORTS 4
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3231
#define ORD_RYS 5
#define NUM_ERIS_PORTS 4
#define BP_FIFO_COUNT 6
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3232
#define ORD_RYS 6
#define NUM_ERIS_PORTS 4
#define BP_FIFO_COUNT 6
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#endif

#if AM_ABCD == 3300
#define ORD_RYS 4
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 1
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#define NO_FRP_FB
#define NO_FRP_CS
#endif

#if AM_ABCD == 3310
#define ORD_RYS 4
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#define NO_FRP_CS
#endif

#if AM_ABCD == 3311
#define ORD_RYS 5
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#define NO_FRP_CS
#endif

#if AM_ABCD == 3320
#define ORD_RYS 5
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 1
#define REWIND_PIPELINES 1
#define NO_FRP_FB
#endif

#if AM_ABCD == 3321
#define ORD_RYS 5
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 1
#define NO_FRP_CS
#define NO_FRP_FB
#endif

#if AM_ABCD == 3322
#define ORD_RYS 6
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 0
#define RR_KERNEL_SPLIT 2
#define NO_FRP_CS
#define NO_FRP_FB
#endif

#if AM_ABCD == 3330
#define ORD_RYS 5
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 0
#define NO_FRP_FB
#define NO_FRP_CS
#endif

#if AM_ABCD == 3331
#define ORD_RYS 6
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 0
#define NO_FRP_FB
#define NO_FRP_CS
#endif

#if AM_ABCD == 3332
#define ORD_RYS 6
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 0
#define NO_FRP_FB
#define NO_FRP_CS
#endif

#if AM_ABCD == 3333
#define ORD_RYS 7
#define NUM_ERIS_PORTS 7
#define BP_FIFO_COUNT 3
#define GQ_KERNEL_SPLIT 2
#define AGGRESSIVE_UNROLL 0
#define BP_BYPASS 0
#define REWIND_PIPELINES 0
#define NO_FRP_FB
#define NO_FRP_CS
#endif


// used by makefile to extract macro values
namespace preprocessor_values
{
#define APPEND( name, a ) APPEND_(name, a)
#define APPEND_( name, a ) name##a
    class APPEND( pp_num_eris_ports_, NUM_ERIS_PORTS );
    class APPEND( pp_bp_fifo_count_, BP_FIFO_COUNT );
    class APPEND( pp_gq_kernel_split_, GQ_KERNEL_SPLIT );
    class APPEND( pp_bp_bypass_, BP_BYPASS );
    class APPEND( pp_rr_kernel_split_, RR_KERNEL_SPLIT );
}

#endif //QC_FPGA_PARAM_CONFIG_H
