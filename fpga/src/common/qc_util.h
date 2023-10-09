#ifndef QC_FPGA_QC_UTIL_H
#define QC_FPGA_QC_UTIL_H

#include "constants.h"
#include <array>

namespace qcf
{
    namespace util
    {
        constexpr int extract_nth_digit( int number, int n )
        {
            while( n > 0 )
            {
                number /= 10;
                --n;
            }
            return number % 10;
//            return (number / static_cast<int>(std::pow(10, n - 1))) % 10;
        }

        constexpr int calc_ncg( int am )
        {
            return (am + 1) * (am + 2) >> 1;
        }

        constexpr int calc_num_eris_per_quartet( const int am_abcd[four] )
        {
            return calc_ncg( am_abcd[0] ) * calc_ncg( am_abcd[1] ) * calc_ncg( am_abcd[2] ) * calc_ncg( am_abcd[3] );
        }

        constexpr int calc_ord_rys( const int am_abcd[four] )
        {
            return (am_abcd[0] + am_abcd[1] + am_abcd[2] + am_abcd[3]) / 2 + 1;
        }

        constexpr int ceil_div( int numerator, int denominator )
        {
            return (numerator + denominator - 1) / denominator;
        }

#define __z_log2d(x) (32 - __builtin_clz(x) - 1)
#define __z_log2q(x) (64 - __builtin_clzll(x) - 1)
#define __z_log2(x) (sizeof(__typeof__(x)) > 4 ? __z_log2q(x) : __z_log2d(x))
#define CEILLOG2(x) ((x) < 1 ?  0 : __z_log2((x)-1) + 1)

        constexpr unsigned long next_pow2( unsigned long x )
        {
            return ((x) < 1 ? 1 : ((x) > (1ULL << 63) ? 0 : 1ULL << CEILLOG2( x )));
        }
    }
}

#endif //QC_FPGA_QC_UTIL_H
