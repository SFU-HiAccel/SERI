#ifndef RYS_QUADRATURE_PREPARATION_H
#define RYS_QUADRATURE_PREPARATION_H

#include "common/types.h"
#include "common/repeat.h"
#include "common/parameters.h"
#include "internal_types.h"

using namespace qcf;

void calc_intermediates(
        const fp_t xyz_abcd[four][nxyz],
        const fp_t exp_abcd[four],
        const fp_t zt1,
        const fp_t et1,
        xyz_derived_pt& xyz_derived
)
{

    fp_t xyz_pq[2][nxyz];
#pragma hls array_partition variable=xyz_pq complete dim=0

    XYZ_LOOP:
    for( int_t j = 0; j < nxyz; j++ )
#pragma hls unroll
    {
        xyz_derived[0][j] = xyz_abcd[0][j] - xyz_abcd[1][j]; // AB
        xyz_derived[1][j] = xyz_abcd[2][j] - xyz_abcd[3][j]; // CD
        xyz_pq[0][j] = (exp_abcd[0] * xyz_abcd[0][j] + exp_abcd[1] * xyz_abcd[1][j]) * zt1; // P = (αA + βB) / ζ
        xyz_pq[1][j] = (exp_abcd[2] * xyz_abcd[2][j] + exp_abcd[3] * xyz_abcd[3][j]) * et1; // Q = (γC + δD) / η
        xyz_derived[2][j] = xyz_pq[0][j] - xyz_abcd[0][j];   // PA
        xyz_derived[3][j] = xyz_pq[1][j] - xyz_abcd[2][j];   // QC
        xyz_derived[4][j] = xyz_pq[0][j] - xyz_pq[1][j];   // PQ
    }

}

template<
        int_t ord_rys
>
void calc_tbb_tbc(
        const fp_t rt_Rys[ord_rys],
        const fp_t zt,
        const fp_t et,
        const fp_t ztet1,
        /*const*/ xyz_derived_pt& xyz_derived,
        tb_b_pt& tb_B,
        tb_c_pt& tb_C
)
{
    PREP_COMPUTE_tb_B_tb_C_I:
    for( int_t i = 0; i < ord_rys; i++ )
#pragma HLS unroll
    {
        /* in RDK1983:
         * - A   means ζ
         * - B   means η
         * - x_A means P_x
         * - x_B means Q_x
         * - x_k means C_x
         * - x_i means A_x
         */
        fp_t half_temp = ((fp_t) 0.5) * rt_Rys[i] * ztet1; // t² / (2(ζ + η))
        tb_B[0][i] = half_temp; // B₀₀  in Eq (42)
        tb_B[1][i] = (((fp_t) 0.5) - half_temp * zt) / et; // B₀₁' in Eq (46)
        tb_B[2][i] = (((fp_t) 0.5) - half_temp * et) / zt; // B₁₀  in Eq (43)

        PREP_COMPUTE_tb_B_tb_C_J:
        for( int_t j = 0; j < nxyz; j++ )
#pragma HLS unroll
        {
            fp_t temp = xyz_derived[4][j] * ((fp_t) 2) * half_temp;
            tb_C[0][i][j] = xyz_derived[3][j] + zt * temp; // C₀₀' in Eq (45)
            tb_C[1][i][j] = xyz_derived[2][j] - et * temp; // C₀₀  in Eq (41)
        }
    }
}

void preparation(
        const qp_t& abcd_inp,
        const rys_t& rys_inp,
        xyz_derived_pt& xyz_derived,
        tb_b_pt& tb_b,
        tb_c_pt& tb_c
)
{
#pragma hls inline off

    fp_t zt /* ζ */, zt1 /* 1/ζ */, et /* η */, et1 /* 1/η */, ztet1;  /* 1 / (ζ + η) */
    zt = abcd_inp.exp_abcd[0] + abcd_inp.exp_abcd[1]; // ζ = α + β
    et = abcd_inp.exp_abcd[2] + abcd_inp.exp_abcd[3]; // η = γ + δ
    zt1 = ((fp_t) 1.0) / zt;                 // 1 / ζ
    et1 = ((fp_t) 1.0) / et;                 // 1 / η
    ztet1 = ((fp_t) 1.0) / (zt + et);        // 1 /(ζ + η)

    calc_intermediates( abcd_inp.xyz_abcd, abcd_inp.exp_abcd, zt1, et1, xyz_derived );
    calc_tbb_tbc<ord_rys>( rys_inp.rt, zt, et, ztet1, xyz_derived, tb_b, tb_c );
}

#endif