#ifndef QC_FPGA_PBLOCK_H
#define QC_FPGA_PBLOCK_H

#include <string>

using std::string;

class pblock
{
public:
    explicit pblock( string name ) : name( std::move( name ) ), slr_num( -1 ) {}

    pblock& slr( int slr )
    {
        slr_num = slr;
        return *this;
    }

    int get_slr() const
    {
        return slr_num;
    }

    string get_name() const
    {
        return name;
    }

    bool operator==( const pblock& rhs ) const
    {
        return name == rhs.name &&
               slr_num == rhs.slr_num;
    }

    bool operator!=( const pblock& rhs ) const
    {
        return !(rhs == *this);
    }

    static pblock none()
    {
        return pblock( "" );
    }

private:
    string name;
    int slr_num;
};

#endif //QC_FPGA_PBLOCK_H
