#include "mob_file.h"
#include "common/qc_util.h"
#include <fstream>
#include <iostream>
#include <sstream>

qcf::mob_file::mob_file( const string& filepath ) : filepath( filepath )
{
    std::ifstream mob_ifstream { filepath };
    if( !mob_ifstream.good() )
    {
        std::cerr << "Error: unable to open \"" << filepath << "\" file." << std::endl;
        exit( EXIT_FAILURE );
    }


    int num_atoms;
    parse_line_as_int( num_atoms, mob_ifstream );

    std::string line;
    while( std::getline( mob_ifstream, line ) )
    {
        // if line is empty or is a comment, skip
        if( line.empty() || line[0] == MOB_FILE_COMMENT_CHAR )
            continue;

        std::istringstream str { line };
        std::string symbol; // chemical symbol
        double x, y, z, a;  // coordinates and exponent of GTO
        int l;              // angular momentum of GTO
        // read the Cartesian coordinates
        if( !(str >> symbol >> x >> y >> z) ) break;
        // s-orbital
        if( !(str >> l >> a) ) break;
        la.push_back( l );
        xa.push_back( a );
        xyz.push_back( x / bohr2angs );
        xyz.push_back( y / bohr2angs );
        xyz.push_back( z / bohr2angs );
        // p-orbital
        if( !(str >> l >> a) ) break;
        la.push_back( l );
        xa.push_back( a );
        xyz.push_back( x / bohr2angs );
        xyz.push_back( y / bohr2angs );
        xyz.push_back( z / bohr2angs );
        // d-orbital
        if( !(str >> l >> a) ) break;
        la.push_back( l );
        xa.push_back( a );
        xyz.push_back( x / bohr2angs );
        xyz.push_back( y / bohr2angs );
        xyz.push_back( z / bohr2angs );
        // f-orbital
        if( !(str >> l >> a) ) break;
        la.push_back( l );
        xa.push_back( a );
        xyz.push_back( x / bohr2angs );
        xyz.push_back( y / bohr2angs );
        xyz.push_back( z / bohr2angs );
    }
    mob_ifstream.close();
}

void qcf::mob_file::parse_line_as_int( int& num_atoms, std::ifstream& mob_ifstream )
{
    string line;
    if( std::getline( mob_ifstream, line ) )
    {
        std::istringstream iss( line );
        if( !(iss >> num_atoms) )
        {
            std::cerr << "Error: First line is not an integer." << std::endl;
            exit( EXIT_FAILURE );
        }
    } else
    {
        std::cerr << "Error: Could not read the first line." << std::endl;
        exit( EXIT_FAILURE );
    }
}

qcf::mob_quartet_info_t qcf::mob_file::calc_info( const int am_abcd[4] ) const
{
    mob_quartet_info_t q_info {};
    q_info.num_quartets = 1;
    q_info.num_cart_eris_per_quartet = qcf::util::calc_num_eris_per_quartet( am_abcd );

    for( int i = 0; i < four; i++ )
    {
        // loop through all basis functions
        for( size_t j = 0; j < la.size(); j++ )
        {
            // if the angular momentum of the basis function matches
            if( am_abcd[i] == la.at( j ) )
                q_info.list_abcd[i].push_back( j );  // store the index of the basis function
        }

        q_info.num_quartets *= q_info.list_abcd[i].size();
    }

    q_info.num_cart_eris = q_info.num_cart_eris_per_quartet * q_info.num_quartets;

    return q_info;
}

const std::vector<double>& qcf::mob_file::get_xyz() const
{
    return xyz;
}

const std::vector<int>& qcf::mob_file::get_la() const
{
    return la;
}

const std::vector<double>& qcf::mob_file::get_xa() const
{
    return xa;
}

const string& qcf::mob_file::get_filepath() const
{
    return filepath;
}
