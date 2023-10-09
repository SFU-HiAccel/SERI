#ifndef QC_FPGA_QC_UTIL_ADVANCED_H
#define QC_FPGA_QC_UTIL_ADVANCED_H

#include "qc_util.h"
#include <tuple>

namespace qcf
{
    namespace util
    {
        struct gq_index_t
        {
            int d;
            int c;
            int b;
            int a;
            int x;

            constexpr bool operator==( const gq_index_t& other ) const
            {
                return (d == other.d) && (c == other.c) && (b == other.b) && (a == other.a) && (x == other.x);
            }
        };

        struct gq_seg_index_t
        {
            int b;
            int a;
            int x;

            constexpr gq_seg_index_t& operator=( const gq_seg_index_t& other )
            {
                if( this != &other )
                {
                    b = other.b;
                    a = other.a;
                    x = other.x;
                }
                return *this;
            }

            constexpr bool operator==( const gq_seg_index_t& other ) const
            {
                return (b == other.b) && (a == other.a) && (x == other.x);
            }
        };

        template<unsigned int len>
        struct gq_index_array_t
        {
            gq_index_t arr[len];
        };

        template<unsigned int len>
        struct gq_seg_index_array_t
        {
            gq_seg_index_t arr[len];
        };

        template<int ncg_d, int ncg_c, int ncg_b, int ncg_a>
        constexpr gq_index_array_t<ncg_d * ncg_c * ncg_b * ncg_a * 3> collect_gq_accessed_indices( int am_d, int am_c, int am_b, int am_a )
        {
            gq_index_array_t<ncg_d * ncg_c * ncg_b * ncg_a * 3> accessed_indices = {};
            int k_a[3] = {};
            int k_b[3] = {};
            int k_c[3] = {};
            int k_d[3] = {};

            int i = 0;
            int i_d = 0, j_d = 0;
            for( int igd = 0; igd < ncg_d; igd++ )
            {
                k_d[0] = am_d - i_d;
                k_d[1] = i_d - j_d;
                k_d[2] = j_d;
                int i_c = 0, j_c = 0;
                for( int igc = 0; igc < ncg_c; igc++ )
                {
                    k_c[0] = am_c - i_c;
                    k_c[1] = i_c - j_c;
                    k_c[2] = j_c;
                    int i_b = 0, j_b = 0;
                    for( int igb = 0; igb < ncg_b; igb++ )
                    {
                        k_b[0] = am_b - i_b;
                        k_b[1] = i_b - j_b;
                        k_b[2] = j_b;
                        int i_a = 0, j_a = 0;
                        for( int iga = 0; iga < ncg_a; iga++ )
                        {
                            k_a[0] = am_a - i_a;
                            k_a[1] = i_a - j_a;
                            k_a[2] = j_a;

                            accessed_indices.arr[i++] = { k_d[0], k_c[0], k_b[0], k_a[0], 0 };
                            accessed_indices.arr[i++] = { k_d[1], k_c[1], k_b[1], k_a[1], 1 };
                            accessed_indices.arr[i++] = { k_d[2], k_c[2], k_b[2], k_a[2], 2 };

                            j_a++;
                            if( j_a > i_a )
                            {
                                i_a++;
                                j_a = 0;
                            }
                        }
                        j_b++;
                        if( j_b > i_b )
                        {
                            i_b++;
                            j_b = 0;
                        }
                    }
                    j_c++;
                    if( j_c > i_c )
                    {
                        i_c++;
                        j_c = 0;
                    }
                }
                j_d++;
                if( j_d > i_d )
                {
                    i_d++;
                    j_d = 0;
                }
            }
            return accessed_indices;
        }

        template<int ncg_b, int ncg_a>
        constexpr gq_seg_index_array_t<ncg_b * ncg_a * 3> collect_gq_segment_accessed_indices( int am_b, int am_a )
        {
            gq_seg_index_array_t<ncg_b * ncg_a * 3> accessed_indices = {};
            int k_a[3] = {};
            int k_b[3] = {};

            int i = 0;
            int i_b = 0, j_b = 0;
            for( int igb = 0; igb < ncg_b; igb++ )
            {
                k_b[0] = am_b - i_b;
                k_b[1] = i_b - j_b;
                k_b[2] = j_b;
                int i_a = 0, j_a = 0;
                for( int iga = 0; iga < ncg_a; iga++ )
                {
                    k_a[0] = am_a - i_a;
                    k_a[1] = i_a - j_a;
                    k_a[2] = j_a;

                    accessed_indices.arr[i++] = { k_b[0], k_a[0], 0 };
                    accessed_indices.arr[i++] = { k_b[1], k_a[1], 1 };
                    accessed_indices.arr[i++] = { k_b[2], k_a[2], 2 };

                    j_a++;
                    if( j_a > i_a )
                    {
                        i_a++;
                        j_a = 0;
                    }
                }
                j_b++;
                if( j_b > i_b )
                {
                    i_b++;
                    j_b = 0;
                }
            }

            return accessed_indices;
        }

        template<typename T, unsigned int len>
        struct partial_array_t
        {
            T elements[len];
            unsigned int count;
        };

        template<typename T, unsigned int len>
        constexpr partial_array_t<T, len> get_unique_elements( const T (& arr)[len] )
        {
            partial_array_t<T, len> result {};
            unsigned int unique_count = 0;

            for( unsigned int i = 0; i < len; ++i )
            {
                bool is_unique = true;
                for( unsigned int j = 0; j < i; ++j )
                {
                    if( arr[i] == arr[j] )
                    {
                        is_unique = false;
                        break;
                    }
                }
                if( is_unique )
                {
                    result.elements[unique_count++] = arr[i];
                }
            }

            result.count = unique_count;
            return result;
        }

        template<int ncg_b, int ncg_a>
        constexpr unsigned int calc_segment_unique_len( int am_b, int am_a )
        {
            gq_seg_index_array_t<ncg_b * ncg_a * 3> indices = collect_gq_segment_accessed_indices<ncg_b, ncg_a>( am_b, am_a );
            return get_unique_elements<gq_seg_index_t, ncg_b * ncg_a * 3>( indices.arr ).count;
        }

        template<int ncg_b, int ncg_a>
        constexpr partial_array_t<gq_seg_index_t, ncg_b * ncg_a * 3> collect_gq_segment_unique_indices( int am_b, int am_a )
        {
            gq_seg_index_array_t<ncg_b * ncg_a * 3> indices = collect_gq_segment_accessed_indices<ncg_b, ncg_a>( am_b, am_a );
            return get_unique_elements<gq_seg_index_t, ncg_b * ncg_a * 3>( indices.arr );
        }

    }
}

#endif //QC_FPGA_QC_UTIL_ADVANCED_H
