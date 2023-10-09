#ifndef RYS_QUADRATURE_COMPRESS_STORE_SPLIT_H
#define RYS_QUADRATURE_COMPRESS_STORE_SPLIT_H

#include "common/types.h"
#include <limits>
#include <algorithm>
#include <ap_fixed.h>
#include "common/repeat.h"
#include "common/parameters.h"
#include "common/parameters.h"


using namespace qcf;

fp_t calc_epsilon_inv( const fp_t b_max, fp_t* epsilon )
{
    constexpr fp_t s_val = static_cast<fp_t>((0b1UL << (eri_bits - 1)) - 1);
    constexpr fp_t s_val_inv = (fp_t) 1 / static_cast<fp_t>((0b1UL << (eri_bits - 1)) - 1);
    fp_t e1;
    if( b_max == 0 ) // is this needed?
    {
        *epsilon = std::numeric_limits<fp_t>::min();
        e1 = 1;
    } else
    {
        *epsilon = b_max * s_val_inv; // b_max / s_val
        e1 = s_val * ((fp_t) 1 / b_max); // s_val / s_val
    }
    return e1;
}

template<int pack_num>
packed_eri_t pack_and_compress_eri( const fp_t val_buf[pack_num], fp_t e1 )
{
#pragma hls inline
    packed_eri_t p_eri = {};
#pragma hls aggregate variable=p_eri

    for( int i = 0; i < pack_num; ++i )
#pragma hls unroll
    {
        fp_t t = val_buf[i] * e1;
//        fp_t r = t > 0.0 ? t + (fp_t) 0.5 : t - (fp_t) 0.5;
        ap_ufixed<16, 16, AP_RND_CONV, AP_WRAP> r = t;
        p_eri.data[i] = (short) r;
    }
    return p_eri;
}


#endif