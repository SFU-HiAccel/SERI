#ifndef QC_FPGA_MOB_FILE_H
#define QC_FPGA_MOB_FILE_H

#include <string>
#include <vector>
#include "common/constants.h"

using std::string;

namespace qcf
{
    static const char MOB_FILE_COMMENT_CHAR = '#';

    struct mob_quartet_info_t
    {
        /// contains the indices for the basis functions that match the specified angular momentum for a, b, c and d
        std::vector<size_t> list_abcd[four];
        /// number of ERIs per quartet
        size_t num_cart_eris_per_quartet;
        /// total number of quartets in the molecule
        size_t num_quartets;
        /// total number of ERIs in the molecule
        size_t num_cart_eris;
    };

    class mob_file
    {
    public:
        explicit mob_file( const string& filepath );
        mob_quartet_info_t calc_info( const int am_abcd[4] ) const;
        const string& get_filepath() const;
        const std::vector<double>& get_xyz() const;
        const std::vector<int>& get_la() const;
        const std::vector<double>& get_xa() const;

    private:
        std::string filepath;
        /// cartesian coordinates for GTOs
        std::vector<double> xyz;
        /// angular momentum of GTOs
        std::vector<int> la;
        /// exponents of GTOs
        std::vector<double> xa;

        static void parse_line_as_int( int& num_atoms, std::ifstream& mob_ifstream );
    };
}

#endif //QC_FPGA_MOB_FILE_H
