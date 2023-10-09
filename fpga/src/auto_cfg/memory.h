#ifndef QC_FPGA_MEMORY_H
#define QC_FPGA_MEMORY_H

#include <string>

using std::string;

class memory
{
public:
    virtual string string_def() const = 0;
    virtual memory* clone() const = 0;
protected:
    static string range_to_string( unsigned int from, int to )
    {
        string s = "[" + std::to_string( from );
        if( to > 0 ) s += ":" + std::to_string( to );
        s += "]";
        return s;
    }
};

class hbm : public memory
{
public:
    explicit hbm( unsigned int pc ) : pc_from( pc ), pc_to( -1 ), s_axi_port( -1 ), using_rama( false ) {}

    hbm( unsigned int pc_from, unsigned int pc_to ) : pc_from( pc_from ), pc_to( (int) pc_to ), s_axi_port( -1 ), using_rama( false ) {}

    hbm& saxi( int s_axi_port )
    {
        this->s_axi_port = s_axi_port;
        return *this;
    }

    hbm& rama()
    {
        this->using_rama = true;
        return *this;
    }

    string string_def() const override
    {
        string s = "HBM" + range_to_string( pc_from, pc_to );
        if( s_axi_port > 0 ) s += "." + std::to_string( s_axi_port );
        if( using_rama ) s += ".RAMA";

        if( using_rama && s_axi_port < 0 )
            fprintf( stderr, "WARNING: Memory definition '%s' specifies RAMA, but does not specify an s_axi port.\n", s.c_str() );
        return s;
    }

    memory* clone() const override
    {
        return new hbm( *this );
    };

private:
    unsigned int pc_from;
    int pc_to;
    int s_axi_port;
    bool using_rama;
};

class ddr : public memory
{
public:
    explicit ddr( unsigned int bank ) : bank_from( bank ), bank_to( -1 ) {}

    ddr( unsigned int bank_from, unsigned int bank_to ) : bank_from( bank_from ), bank_to( (int) bank_to ) {}

    string string_def() const override
    {
        return "DDR" + range_to_string( bank_from, bank_to );
    }

    memory* clone() const override
    {
        return new ddr( *this );
    };

private:
    unsigned int bank_from;
    int bank_to;
};

#endif //QC_FPGA_MEMORY_H
