#ifndef QC_FPGA_QC_HOST_UTIL_H
#define QC_FPGA_QC_HOST_UTIL_H

#include <string>
#include "common/constants.h"

using std::string;

namespace qcf
{
    namespace util
    {
        namespace
        {
            string s_am[] { "s", "p", "d", "f" };
        }

        string quartet_simple_name( const int am_abcd[four] )
        {
            return s_am[am_abcd[0]] + s_am[am_abcd[1]] + s_am[am_abcd[2]] + s_am[am_abcd[3]];
        }

        string quartet_friendly_name( const int am_abcd[four] )
        {
            return "[" + s_am[am_abcd[0]] + s_am[am_abcd[1]] + "|" + s_am[am_abcd[2]] + s_am[am_abcd[3]] + "]";
        }

    }
}

#endif //QC_FPGA_QC_HOST_UTIL_H
