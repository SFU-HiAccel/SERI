#include <fstream>
#include <iostream>
#include "bin_file.h"

void qcf::bin_file::write_file( const string& filepath, const void* data, unsigned long data_size_bytes )
{
    std::ofstream out_file { filepath, std::ios_base::binary };
    if( !out_file.good() )
    {
        std::cerr << "Error: unable to create or open \"" << filepath << "\" file." << std::endl;
        exit( EXIT_FAILURE );
    }
    out_file.write( reinterpret_cast<const char*>(data), (std::streamsize) data_size_bytes );
    out_file.close();
}

void* qcf::bin_file::read_file( const string& filepath, size_t& data_size_bytes )
{
    std::ifstream in_file { filepath, std::ios_base::binary | std::ios_base::ate };
    if( !in_file.good() )
    {
        std::cerr << "Error: unable to open \"" << filepath << "\" file for reading." << std::endl;
        exit( EXIT_FAILURE );
    }

    data_size_bytes = (size_t) in_file.tellg();
    void* buffer = malloc( data_size_bytes );
    if( buffer == nullptr )
    {
        std::cerr << "Error: unable to allocate memory for data from \"" << filepath << "\" file." << std::endl;
        exit( EXIT_FAILURE );
    }

    in_file.seekg( 0 );
    in_file.read( reinterpret_cast<char*>(buffer), (std::streamsize) data_size_bytes );
    in_file.close();

    return buffer;
}