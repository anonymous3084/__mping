// This file is part of xxxxx-1.
//
// Copyright 2023 The xxxxx-1 Authors
//
// xxxxx-1 is free software : you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
// version. xxxxx-1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with xxxxx-1.  If not, see
// <https://www.gnu.org/licenses/>.

#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

#include <mpi.h>

#include "helpers_for_examples.hpp"
#include "xxxxx-1ing/checking_casts.hpp"
#include "xxxxx-1ing/collectives/alltoall.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/data_buffer.hpp"
#include "xxxxx-1ing/environment.hpp"
#include "xxxxx-1ing/measurements/printer.hpp"
#include "xxxxx-1ing/measurements/timer.hpp"
#include "xxxxx-1ing/named_parameters.hpp"

int main() {
    using namespace xxxxx-1ing;

    xxxxx-1ing::Environment e;
    Communicator         comm;
    std::vector<int>     input(comm.size());
    std::iota(input.begin(), input.end(), 0);
    std::vector<int> output;

    auto sleep_some_time = [&]() {
        static std::mt19937                gen(static_cast<std::mt19937::result_type>(comm.rank() + 17) * 1001);
        std::uniform_int_distribution<int> distrib(50, 10'000);
        const std::chrono::microseconds    sleep_duration{distrib(gen)};
        std::this_thread::sleep_for(sleep_duration);
    };
    // Get timer singleton. Alternatively you can also instantiate a new timer.
    auto& t = xxxxx-1ing::measurements::timer();
    // Synchronizes all ranks via a call to MPI_Barrier and then start the measurement with key "algorithm".
    t.synchronize_and_start("algorithm");
    for (size_t i = 0; i < 3; ++i) {
        t.synchronize_and_start("round" + std::to_string(i));
        {
            // Synchronizes all ranks and then start a measurement with key "preprocessing". The enclosing measurement
            // "algorithm" is still active.
            t.synchronize_and_start("preprocessing");
            sleep_some_time();
            // Stops the measurement "preprocessing". As no communicator wide duration aggregation operation is given
            // the default "max" is chosen to be applied later during evaluation.
            t.stop();

            t.synchronize_and_start("core_algorithm");
            for (size_t j = 0; j < 5u; ++j) {
                // Starts the measurement "subroutine" without synchronizing all ranks
                t.start("subroutine");
                sleep_some_time();
                // Stops the measurement "subroutine" and specifies that the communicator wide duration aggregation
                // operation "min", "max" and "gather" will be applied to this measurement.
                // Furthermore, the measured durations are stored in a list instead of being summed up (- note that this
                // measurement with key "subroutine" will be executed 3*5 times)
                t.stop_and_append(
                    {measurements::GlobalAggregationMode::min,
                     measurements::GlobalAggregationMode::max,
                     measurements::GlobalAggregationMode::gather}
                );
            }
            t.stop();
            t.synchronize_and_start("preprocessing");
            sleep_some_time();
            t.stop();
        }
        t.stop_and_append();
    }
    t.stop();
    // Evaluates the timer and prints the aggregated duration using the print() method from SimpleJsonPrinter.
    t.aggregate_and_print(xxxxx-1ing::measurements::SimpleJsonPrinter{});
    std::cout << std::endl;
    t.aggregate_and_print(xxxxx-1ing::measurements::FlatPrinter{});
    std::cout << std::endl;

    return 0;
}
