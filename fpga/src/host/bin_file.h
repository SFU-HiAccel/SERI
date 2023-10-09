#ifndef QC_FPGA_BIN_FILE_H
#define QC_FPGA_BIN_FILE_H

#include <string>

using std::string;

namespace qcf
{
    class bin_file
    {
    public:
        static void write_file(const string& filepath, const void* data, unsigned long data_size_bytes);
        static void* read_file(const string& filepath, size_t& data_size_bytes);
    };
}


#endif //QC_FPGA_BIN_FILE_H
