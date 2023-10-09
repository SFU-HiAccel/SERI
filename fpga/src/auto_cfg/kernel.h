#ifndef QC_FPGA_KERNEL_H
#define QC_FPGA_KERNEL_H

#include <utility>
#include <stdexcept>
#include <vector>
#include <string>
#include "memory.h"
#include "pblock.h"

using std::string;
using std::vector;

struct kernel_data;

class system_connectivity;

kernel_data* get_kernel_by_id( system_connectivity* sc, unsigned int kernel_id );

struct kernel_inst_data
{
    system_connectivity* sc;
    unsigned int k_id; // cannot just store pointer to kernel_data because its in a vector that can move as it re-sizes itself.
    unsigned int inst_index;
    int slr;
    pblock pb;
};

struct kernel_data
{
    string name;
    int default_slr_num;
    unsigned int num_instances;
    vector<kernel_inst_data> instances;
};

string kernel_inst_string_def( const kernel_inst_data& i_data )
{
    const kernel_data& k_data = *get_kernel_by_id( i_data.sc, i_data.k_id );
    return k_data.name + "_" + std::to_string( i_data.inst_index + 1 );
}

string cfg_slr_string( const kernel_inst_data& i_data )
{
    if( i_data.slr < 0 )
        throw std::invalid_argument( "Cannot generate SLR cfg string for kernel instance '" + kernel_inst_string_def( i_data ) + "' because no SLR has been specified for this instance" );
    return kernel_inst_string_def( i_data ) + ":SLR" + std::to_string( i_data.slr );
}

string cfg_nk_string( const kernel_data& k_data )
{
    return k_data.name + ":" + std::to_string( k_data.num_instances );
}


class kernel;

class kernel_inst
{
    friend system_connectivity;
    friend kernel;
public:
    kernel_inst& slr( int slr )
    {
        kernel_data* k_data = get_kernel_data();
        kernel_inst_data* i_data = get_kernel_inst_data();
        i_data->slr = slr;
        return *this;
    }

    kernel_inst& pblock( pblock pb )
    {
        slr( pb.get_slr() );
        kernel_data* k_data = get_kernel_data();
        kernel_inst_data* i_data = get_kernel_inst_data();
        i_data->pb = pb;
        return *this;
    }

    kernel_inst_data* get_kernel_inst_data() const
    {
        kernel_data* k_data = get_kernel_data();
        if( kernel_inst_id >= k_data->num_instances )
            throw std::out_of_range( "Internal Error: Attempting to get non-existent kernel instance data for kernel '" + k_data->name + "' instance '" + std::to_string( kernel_inst_id ) + "'" );
        return &k_data->instances[kernel_inst_id];
    }

    kernel_data* get_kernel_data() const;

private:
    kernel_inst( system_connectivity* sc, unsigned int kernel_id, unsigned int kernel_inst_id ) :
            kernel_inst_id( kernel_inst_id ),
            kernel_id( kernel_id ),
            sc( sc ) {}

    unsigned int kernel_inst_id;
    unsigned int kernel_id;
    system_connectivity* sc;
};


class kernel
{
    friend system_connectivity;
public:
    static kernel null()
    {
        return { nullptr, INT32_MAX };
    }

    kernel& num( int n )
    {
        kernel_data* k_data = get_kernel_data();
        if( n < k_data->num_instances )
            fprintf( stderr, "WARNING: Number of instances for kernel '%s' set to '%d' which is lower than the current '%d'. Only set the number of kernel instances once, when it is first defined.\n", k_data->name.c_str(), n, k_data->num_instances );

        k_data->num_instances = n;
        while( k_data->instances.size() < n )
            k_data->instances.push_back( { .sc = sc, .k_id = kernel_id, .inst_index = (unsigned int) k_data->instances.size(), .slr = k_data->default_slr_num, .pb = pblock::none() } );
        return *this;
    }

    kernel& default_slr( int slr )
    {
        kernel_data* k_data = get_kernel_data();
        k_data->default_slr_num = slr;

        // apply default to all already allocated instances that haven't been explicitly assigned an SLR
        for( kernel_inst_data& i_data: k_data->instances )
            if( i_data.slr < 0 ) i_data.slr = slr;

        if( slr < 0 )
            fprintf( stderr, "WARNING: Default SLR for kernel '%s' was set to a negative number ('%d'). Negative slr numbers are treated as unassigned.\n", k_data->name.c_str(), slr );
        return *this;
    }

    kernel& default_pblock( pblock pb )
    {
        default_slr( pb.get_slr() );
        kernel_data* k_data = get_kernel_data();
        for( kernel_inst_data& i_data: k_data->instances )
            if( i_data.pb == pblock::none() ) i_data.pb = pb;
        return *this;
    }

    kernel_inst operator[]( unsigned int index ) const
    {
        if( sc == nullptr )
            throw std::invalid_argument( "A null kernel cannot be indexed" );
        if( index >= get_kernel_data()->num_instances )
            throw std::out_of_range( "Index [" + std::to_string( index ) + "] of '" + get_kernel_data()->name + "' is out of range. Defined number of instances for this kernel is '" + std::to_string( get_kernel_data()->num_instances ) + "'" );

        return { sc, kernel_id, index };
    }

    kernel_data* get_kernel_data() const;


private:
    kernel( system_connectivity* sc, unsigned int kernel_id ) :
            kernel_id( kernel_id ),
            sc( sc ) {}

    unsigned int kernel_id;
    system_connectivity* sc;
};


#endif //QC_FPGA_KERNEL_H
