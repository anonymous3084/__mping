// This example demonstrates how to use the upcoming User-Level Failure-Mitigation (ULFM) feature of MPI. This example
// and the accompanying plugin is tested with OpenMPI 5.0.2 See:
// https://docs.open-mpi.org/en/v5.0.x/features/ulfm.html#ulfm-label
//
// In order to run this example, you have to:
// (1) Compile OpenMPI >= 5.0 with the `--with-ft=mpi` flag.
// (2) Include the installation dir into your `LIBRARY_PATH`, `LD_LIBRARY_PATH`, `PATH`, and `CPATH`.
// (3) compile this example by passing `-Dxxxxx-1_ENABLE_ULFM=On` to `cmake`.
// (4a) Run the binary using `mpirun -n <n> --with-ft=mpi ...`.
// (4b) Send a SIGKILL to one of the processes.

#include "xxxxx-1ing/plugin/ulfm.hpp"

#include "xxxxx-1ing/checking_casts.hpp"
#include "xxxxx-1ing/collectives/allreduce.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/environment.hpp"

using namespace ::xxxxx-1ing;

int main(int argc, char** argv) {
    constexpr int root = 0;

    // Call MPI_Init() and MPI_Finalize() automatically.
    Environment<> env(argc, argv);

    Communicator<std::vector, plugin::UserLevelFailureMitigation> comm;

    int result = 0;
    while (true) {
        try {
            comm.allreduce(send_recv_buf(result), op(xxxxx-1ing::ops::plus<>()));
            KASSERT(!comm.is_revoked());
        } catch ([[maybe_unused]] MPIFailureDetected const& _) {
            if (!comm.is_revoked()) {
                comm.revoke();
            }
            comm = comm.shrink();
            if (comm.rank() == root) {
                std::cerr << "Process failure detected and recovered from. Remaining ranks: " << comm.size()
                          << std::endl;
            }
        }
    }

    return EXIT_SUCCESS;
}
