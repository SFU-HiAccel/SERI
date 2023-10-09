#ifndef QC_FPGA_CONNECTIVITY_H
#define QC_FPGA_CONNECTIVITY_H

#include <utility>
#include <stdexcept>
#include <vector>
#include <string>
#include "memory.h"
#include "kernel.h"
#include "connection.h"

using std::string;
using std::vector;
using std::tuple;

struct memory_connection_info
{
    kernel_inst krnl;
    bundle bndl;
    memory* mem; // technically will leak, but I don't care about it since program lifetime is extremely short
};

struct stream_connection_info
{
    kernel_inst k_from;
    bundle bndl;
    kernel_inst k_to;
};

vector<string> cfg_sp_memory_strings( const memory_connection_info& m_conn_info )
{
    vector<string> sp_defs;
    const kernel_inst_data& i_data = *m_conn_info.krnl.get_kernel_inst_data();

    for( const string& conn_name: m_conn_info.bndl.raw_port_strings() )
    {
        memory* mem = m_conn_info.mem;
        if( mem == nullptr ) throw std::invalid_argument( "Internal Error: Missing memory object for kernel instance '" + kernel_inst_string_def( i_data ) + "' on port '" + conn_name + "'" );
        sp_defs.push_back( kernel_inst_string_def( i_data ) + "." + conn_name + ":" + mem->string_def() );
    }
    return sp_defs;
}

vector<string> cfg_sc_stream_strings( const stream_connection_info& s_conn_info )
{
    vector<string> sc_defs;
    const kernel_inst_data& i_data_from = *s_conn_info.k_from.get_kernel_inst_data();
    const kernel_inst_data& i_data_to = *s_conn_info.k_to.get_kernel_inst_data();

    for( const tuple<string, string, unsigned int>& conn_name_pair: s_conn_info.bndl.port_pair_strings() )
        sc_defs.push_back( kernel_inst_string_def( i_data_from ) + "." + std::get<0>( conn_name_pair ) + ":" + kernel_inst_string_def( i_data_to ) + "." + std::get<1>( conn_name_pair ) + ":" + std::to_string( std::get<2>( conn_name_pair ) ) );
    return sc_defs;
}

class system_connectivity
{
    friend kernel_data* kernel::get_kernel_data() const;
    friend kernel_data* kernel_inst::get_kernel_data() const;
public:
    system_connectivity( string cfg_filepath, string pre_place_tcl_filepath ) :
            cfg_filepath( std::move( cfg_filepath ) ),
            pre_place_tcl_filepath( std::move( pre_place_tcl_filepath ) ) {}

    kernel def_kernel( const string& name )
    {
        kernel_datas.push_back(
                {
                        .name = name,
                        .default_slr_num = -1,
                        .num_instances = 0,
                        .instances = vector<kernel_inst_data>(),
                }
        );
        kernel k( this, (unsigned int) (kernel_datas.size() - 1) );
        return k.num( 1 ); // num will add kernel instance to vector
    }

    void add_tcl( const string& tcl_code )
    {
        extra_tcl.push_back( tcl_code );
    }

    void finalize()
    {
        FILE* cfg_file = fopen( cfg_filepath.c_str(), "a" );
        if( !cfg_file )
            throw std::runtime_error( "Failed to open configuration file for writing: " + cfg_filepath );
        cfg_out( cfg_file );
        fclose( cfg_file );

        FILE* tcl_file = fopen( pre_place_tcl_filepath.c_str(), "a" );
        if( !tcl_file )
            throw std::runtime_error( "Failed to open TCL file for writing: " + pre_place_tcl_filepath );
        pre_place_tcl_out( tcl_file );
        fclose( tcl_file );
    }

    void cfg_out( FILE* fd )
    {
        // generate kernel and kernel instance configs
        for( const kernel_data& k_data: kernel_datas )
        {
            fprintf( fd, "nk=%s\n", cfg_nk_string( k_data ).c_str() );
            for( const kernel_inst_data& i_data: k_data.instances )
                if( i_data.slr >= 0 ) fprintf( fd, "slr=%s\n", cfg_slr_string( i_data ).c_str() );
        }

        fprintf( fd, "\n" );

        // generate memory port connection configs
        for( const memory_connection_info& m_conn_info: m_conn_infos )
            for( const string& conn_str: cfg_sp_memory_strings( m_conn_info ) )
                fprintf( fd, "sp=%s\n", conn_str.c_str() );

        fprintf( fd, "\n" );

        // generate stream port connection configs
        for( const stream_connection_info& s_conn_info: s_conn_infos )
            for( const string& conn_str: cfg_sc_stream_strings( s_conn_info ) )
                fprintf( fd, "sc=%s\n", conn_str.c_str() );
    }

    void pre_place_tcl_out( FILE* fd )
    {
        for( const string& tcl_code: extra_tcl )
            fprintf( fd, "%s\n", tcl_code.c_str() );

        for( const kernel_data& k_data: kernel_datas )
        {
            for( const kernel_inst_data& i_data: k_data.instances )
                if( i_data.pb != pblock::none() ) fprintf( fd, "add_cells_to_pblock %s [get_cells -hierarchical -filter \"NAME=~*%s*\"]\n", i_data.pb.get_name().c_str(), kernel_inst_string_def( i_data ).c_str() );
        }
    }

    static void add_kernel_connection( const stream_connection_info& conn_info )
    {
        conn_info.k_from.sc->s_conn_infos.push_back( conn_info );
    }

    static void add_memory_connection( const memory_connection_info& conn_info )
    {
        conn_info.krnl.sc->m_conn_infos.push_back( conn_info );
    }

    kernel_data* get_kernel_by_id( unsigned int kernel_id )
    {
        if( kernel_id >= kernel_datas.size() )
            throw std::out_of_range( "Internal Error: Attempting to get non-existent kernel data for kernel_id '" + std::to_string( kernel_id ) + "'" );
        return &kernel_datas[kernel_id];
    }

private:
    string cfg_filepath;
    string pre_place_tcl_filepath;
    vector<string> extra_tcl;
    vector<kernel_data> kernel_datas;
    vector<stream_connection_info> s_conn_infos;
    vector<memory_connection_info> m_conn_infos;
};

kernel_data* get_kernel_by_id( system_connectivity* sc, unsigned int kernel_id )
{
    return sc->get_kernel_by_id( kernel_id );
}

kernel_data* kernel::get_kernel_data() const
{
    if( sc == nullptr )
        throw std::invalid_argument( "An operation was attempted on a null kernel. This is not allowed." );
    return sc->get_kernel_by_id( kernel_id );
}

kernel_data* kernel_inst::get_kernel_data() const
{
    return sc->get_kernel_by_id( kernel_id );
}

// -------------------------------------------------------------
// overload operators to allow convenient connection definitions
// -------------------------------------------------------------

memory_connection_info operator<<( const kernel_inst& krnl, const bundle& bndl )
{
    return { .krnl = krnl, .bndl = bndl, .mem = nullptr };
}

memory_connection_info operator<<( const kernel_inst& krnl, const string& connection_name )
{
    return krnl << connection( connection_name );
}

void operator<<( memory_connection_info conn_info, const memory& mem )
{
    conn_info.mem = mem.clone();
    system_connectivity::add_memory_connection( conn_info );
}

stream_connection_info operator>>( const kernel_inst& krnl, const bundle& bndl )
{
    return { .k_from = krnl, .bndl =  bndl, .k_to = krnl };
}

stream_connection_info operator>>( const kernel_inst& krnl, const string& connection_name )
{
    return krnl >> connection( connection_name );
}

kernel_inst operator>>( stream_connection_info conn_info, const kernel_inst& krnl )
{
    conn_info.k_to = krnl;
    system_connectivity::add_kernel_connection( conn_info );
    return krnl;
}

#endif //QC_FPGA_CONNECTIVITY_H```