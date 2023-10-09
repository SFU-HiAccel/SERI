#ifndef QC_FPGA_MDIM_RANGE_H
#define QC_FPGA_MDIM_RANGE_H

#include <array>
#include <tuple>
#include <cstddef> // for std::size_t

template<std::size_t... Dims>
class mdim_range_test {
public:
    static constexpr std::size_t dimensions[sizeof...(Dims)] = {Dims...};

    class iterator {
    private:
        std::array<int, sizeof...(Dims)> indices{};
        bool end = false;

        template<size_t N>
        bool increment() {
            if constexpr (N == 0) {
                indices[N]++;
                if (indices[N] == dimensions[N]) {
                    end = true;
                    return true;
                }
                return false;
            } else {
                indices[N]++;
                if (indices[N] == dimensions[N]) {
                    indices[N] = 0;
                    return increment<N - 1>();
                }
                return false;
            }
        }

        template<typename Array, size_t... I>
        auto indices_to_tuple( const Array& arr, std::index_sequence<I...>) const {
            return std::make_tuple(arr[I]...);
        }

    public:
        iterator( bool isEnd = false) {
            end = isEnd;
        }

        iterator& operator++() {
            increment<sizeof...(Dims) - 1>();
            return *this;
        }

        bool operator==( const iterator& rhs ) const
        {
            // if both are end, they are equal regardless of indices
            return (end && rhs.end) || (indices == rhs.indices && end == rhs.end);
        }

        bool operator!=( const iterator& rhs ) const
        {
            return !(rhs == *this);
        }

        auto operator*() const {
            return indices_to_tuple( indices, std::make_index_sequence<sizeof...(Dims)> {} );
        }
    };

    iterator begin() const {
        return iterator();
    }

    iterator end() const {
        return iterator( true);
    }
};


#endif //QC_FPGA_MDIM_RANGE_H
