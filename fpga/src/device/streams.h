#ifndef QC_FPGA_STREAMS_H
#define QC_FPGA_STREAMS_H

#ifndef TAPA_FLOW
#define VPP_FLOW
#endif

#ifdef VPP_FLOW
#include <hls_stream.h>
#endif
#ifdef TAPA_FLOW
#include <tapa.h>
#endif

namespace qcf
{
#ifdef VPP_FLOW
    template<typename T>
    using istream = hls::stream<T>;
    template<typename T>
    using ostream = hls::stream<T>;
#endif
#ifdef TAPA_FLOW
    template<typename T>
    using istream = tapa::istream<T>;
    template<typename T>
    using ostream = tapa::ostream<T>;
#endif
}

#endif //QC_FPGA_STREAMS_H
