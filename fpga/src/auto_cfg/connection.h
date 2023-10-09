#ifndef QC_FPGA_CONNECTION_H
#define QC_FPGA_CONNECTION_H

#include <vector>
#include <string>
#include <tuple>

using std::string;
using std::vector;
using std::tuple;

class system_connectivity;

class bundle;

class connection
{
    friend system_connectivity;
public:
    explicit connection( const string& port_name ) :
            raw_name( port_name ),
            from_port( port_name + "_out" ),
            to_port( port_name + "_in" ),
            fifo_depth( 0 ),
            multiplicity( 1 ) {}

    connection( string from_port_name, string to_port_name ) :
            from_port( std::move( from_port_name ) ),
            to_port( std::move( to_port_name ) ),
            fifo_depth( 0 ),
            multiplicity( 1 ) {}

    connection& fifo( unsigned int depth )
    {
        fifo_depth = depth;
        if( __builtin_popcount( fifo_depth ) > 1 )
            fprintf( stderr, "WARNING: Fifo depth '%d' for stream connection '%s' to '%s' is not a power of 2, or zero.\n", fifo_depth, from_port.c_str(), to_port.c_str() );
        if( (fifo_depth != 0 && fifo_depth < 16) || fifo_depth > 32768 )
            fprintf( stderr, "WARNING: Fifo depth '%d' for stream connection '%s' to '%s' is not between 16 and 32768, or zero.\n", fifo_depth, from_port.c_str(), to_port.c_str() );
        return *this;
    }

    connection& count( unsigned int num )
    {
        multiplicity = num;
        return *this;
    }

    vector<string> raw_port_strings() const
    {
        vector<string> strs;
        if( multiplicity == 1 )
            strs.push_back( raw_name );
        else
            for( int i = 0; i < multiplicity; ++i )
                strs.push_back( raw_name + "_" + std::to_string( i ) );
        return strs;
    }

    vector<tuple<string, string, unsigned int>> port_pair_strings() const
    {
        vector<tuple<string, string, unsigned int>> strs;
        if( multiplicity == 1 )
            strs.emplace_back( from_port, to_port, fifo_depth );
        else
            for( int i = 0; i < multiplicity; ++i )
                strs.emplace_back( from_port + "_" + std::to_string( i ), to_port + "_" + std::to_string( i ), fifo_depth );
        return strs;
    }

private:
    string raw_name;
    string from_port;
    string to_port;
    unsigned int fifo_depth;
    unsigned int multiplicity;
};

class connection_to_builder
{
public:
    explicit connection_to_builder( const char* nm ) : name( nm ) {}

    connection operator()( const string& to_port_name )
    {
        return { name, to_port_name };
    }

private:
    string name;
};

connection_to_builder operator "" _to( const char* str, size_t )
{
    return connection_to_builder( str );
}

connection operator "" _( const char* str, size_t )
{
    return connection( str );
}

class bundle
{
public:
    bundle() = default;

    bundle( const connection& conn )
    {
        connections.push_back( conn );
    };

    bundle& add( const string& port_name )
    {
        connections.emplace_back( port_name );
        return *this;
    }

    bundle& add( const connection& conn )
    {
        connections.push_back( conn );
        return *this;
    }

    bundle& add_if( bool condition, const string& port_name )
    {
        if( condition )
            connections.emplace_back( port_name );
        return *this;
    }

    bundle& add_if( bool condition, const connection& conn )
    {
        if( condition )
            connections.push_back( conn );
        return *this;
    }

    vector<string> raw_port_strings() const
    {
        vector<string> conns;
        for( const connection& conn: connections )
        {
            vector<string> v = conn.raw_port_strings();
            conns.insert( conns.end(), v.begin(), v.end() );
        }
        return conns;
    }

    vector<tuple<string, string, unsigned int>> port_pair_strings() const
    {
        vector<tuple<string, string, unsigned int>> conns;
        for( const connection& conn: connections )
        {
            vector<tuple<string, string, unsigned int>> v = conn.port_pair_strings();
            conns.insert( conns.end(), v.begin(), v.end() );
        }
        return conns;
    }


private:
    vector<connection> connections;
};


#endif //QC_FPGA_CONNECTION_H
