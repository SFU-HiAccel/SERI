#ifndef QC_FPGA_TIMER_H
#define QC_FPGA_TIMER_H

#include <chrono>

using std::chrono::steady_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

namespace qcf
{

    class timer
    {
    public:
        timer() : start_ { steady_clock::now() } {}

        double elapsed_ms()
        {
            auto now = steady_clock::now();
            return duration_cast<duration<double, std::milli>>( now - start_ ).count();
        }

    private:
        steady_clock::time_point start_;
    };

}

#endif //QC_FPGA_TIMER_H
