#ifndef QC_FPGA_DATA_COMPARATOR_HPP
#define QC_FPGA_DATA_COMPARATOR_HPP

#include <cstddef>
#include <vector>
#include <functional>

namespace qcf
{
    using std::size_t;

    template<typename T>
    class data_comparator
    {
    public:
        struct elem_info
        {
            T diff;
            size_t index;
            T a;
            T b;
        };

        static std::vector<elem_info> find_above_threshold( T* a, T* b, size_t num_elements, T threshold )
        {
            std::vector<elem_info> flagged_elems;
            compare_arrays( a, b, num_elements, [=]( T x ) { return x > threshold; }, [&]( elem_info i ) { flagged_elems.push_back( i ); } );
            return flagged_elems;
        }

        static elem_info find_max_diff( T* a, T* b, size_t num_elements )
        {
            T max_diff = 0;
            elem_info max_diff_info { 0, 0, 0, 0 };
            compare_arrays( a, b, num_elements, [&]( T x ) { return x > max_diff && (max_diff = x, true); }, [&]( elem_info i ) { max_diff_info = i; } );
            return max_diff_info;
        }

        static void compare_arrays( T* a, T* b, size_t num_elements, std::function<bool( T )> is_flagged, std::function<void( elem_info )> handle_flagged )
        {
            for( size_t i = 0; i < num_elements; ++i )
            {
                T abs_diff = abs( a[i] - b[i] );
                if( is_flagged( abs_diff ) )
                    handle_flagged( { abs_diff, i, a[i], b[i] } );
            }
        }

        static inline T abs( T x )
        {
            return x < 0 ? -x : x;
        }
    };

}

#endif //QC_FPGA_DATA_COMPARATOR_HPP
