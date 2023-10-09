#ifndef QC_FPGA_PACKED_ARRAY_HPP
#define QC_FPGA_PACKED_ARRAY_HPP

#include <vector>
#include <cassert>
#include <tuple>
#include <cstring>
#include "ap_int.h"

using std::size_t;

template<typename T, size_t... Dims>
class packed_array;

// proxy class for intermediate dimensions
template<typename T, typename Array, size_t Burn, size_t D, size_t... Rest>
class array_proxy
{
public:
    array_proxy( Array& arr, size_t idx ) : arr( arr ), index( idx ) {}

    array_proxy<T, Array, D, Rest...> operator[]( size_t idx )
    {
        size_t new_index = index * D + idx;
        return array_proxy<T, Array, D, Rest...>( arr, new_index );
    }

    const T& operator=( const T& value ) // NOLINT(*-unconventional-assign-operator)
    {
        return arr.data[index] = value;
    }

    operator T&() // NOLINT(*-explicit-constructor)
    {
        return arr.data[index];
    }


private:
    Array& arr;
    const size_t index;

};

template<typename T, size_t... Dims>
class packed_array
{
    template<typename T_f, typename Array_f, size_t Burn_f, size_t D_f, size_t... Rest_f>
    friend
    class array_proxy;

public:
    static constexpr size_t size_bytes = sizeof( T ) * (... * Dims);
    static constexpr size_t bit_width = 8 * size_bytes;

    packed_array() = default;
//    {
//#pragma hls array_partition variable=data complete dim=0
//    }

    packed_array( ap_uint<bit_width> uint ) // NOLINT(*-explicit-constructor)
    {
        *reinterpret_cast<ap_uint<bit_width>*>(data) = uint;
    }

    operator ap_uint<bit_width>() // NOLINT(*-explicit-constructor)
    {
        ap_uint<bit_width> uint;
        copy_to( &uint );
        return uint;
    }

    array_proxy<T, packed_array<T, Dims...>, Dims..., 1, 0> operator[]( size_t idx )
    {
        return array_proxy<T, packed_array<T, Dims...>, Dims..., 1, 0>( *this, idx );
    }

    inline void copy_from( const void* src )
    {
        memcpy( data, src, size_bytes );
    }

    inline void copy_to( void* dest )
    {
        memcpy( dest, data, size_bytes );
    }

    inline void set_zero()
    {
        memset( data, 0, size_bytes );
    }


//private:
    T data[(... * Dims)]; // Internal 1D array for storing elements of type T

};

#endif //QC_FPGA_PACKED_ARRAY_HPP
