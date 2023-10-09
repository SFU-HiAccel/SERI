#ifndef QC_FPGA_CONSTANTS_H
#define QC_FPGA_CONSTANTS_H

namespace qcf
{
    /// 4 for [ab|cd]
    constexpr int four = 4;
    /// 3 for x, y, z
    constexpr int nxyz = 3;

    /// 1.0 Bohr to Ångstrom
    constexpr double bohr2angs { 0.5291772086 };
    /// 2/π
    constexpr double two_over_pi { .6366197723675813 };
    /// 2π^(5/2)
    constexpr double two_pi_52 { 34.98683665524973 };
}

#endif //QC_FPGA_CONSTANTS_H
