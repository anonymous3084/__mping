#include <algorithm>
#include <cstddef>
#include <optional>
#include <ostream>

#include <mpi.h>

#include "xxxxx-1ing/collectives/reduce.hpp"
#include "xxxxx-1ing/communicator.hpp"
#include "xxxxx-1ing/environment.hpp"
#include "xxxxx-1ing/mpi_datatype.hpp"
#include "xxxxx-1ing/mpi_ops.hpp"
#include "xxxxx-1ing/named_parameters.hpp"

template <size_t K, typename ValueType>
class TopK {
public:
    explicit TopK() noexcept {}

    ValueType& operator[](size_t i) {
        return elements[i];
    }

    ValueType const& operator[](size_t i) const {
        return elements[i];
    }

private:
    std::array<ValueType, K> elements;
};

template <size_t K, typename ValueType>
TopK<K, ValueType> merge(TopK<K, ValueType> const& lhs, TopK<K, ValueType> const& rhs) {
    size_t          lhs_current = 0;
    size_t          rhs_current = 0;
    TopK<K, size_t> merged;
    for (size_t output_idx = 0; output_idx < K; output_idx++) {
        if (lhs[lhs_current] < rhs[rhs_current]) {
            merged[output_idx] = lhs[lhs_current];
            lhs_current++;
        } else {
            merged[output_idx] = rhs[rhs_current];
            rhs_current++;
        }
    }
    return merged;
}

template <size_t K, typename ValueType>
std::ostream& operator<<(std::ostream& os, TopK<K, ValueType> const& top_k) {
    os << "TopK(";
    for (size_t idx = 0; idx < K; ++idx) {
        if (idx != 0) {
            os << ", ";
        }
        os << top_k[idx];
    }
    os << ")";
    return os;
}

// Top-K reduction using plain MPI
template <size_t K, typename ValueType>
std::optional<TopK<K, ValueType>> mpi_top_k(TopK<K, ValueType> const& local_top_k, MPI_Comm comm) {
    // We first have to create a custom datatype:
    MPI_Datatype top_k_type;

    // We rely on the automatic compile-time deduction of the datatype of the single elements (ValueType) via xxxxx-1 to
    // make this code as generic as possible without adding another 300+ lines of code here.
    // See xxxxx-1ing/mpi_datatype.hpp for the implementation of xxxxx-1ing::mpi_datatype.
    MPI_Type_contiguous(K, xxxxx-1ing::mpi_datatype<ValueType>(), &top_k_type);
    MPI_Type_commit(&top_k_type);

    // Second, we have to register our custom reduce operation with MPI:
    MPI_Op             top_k_merge_op;
    MPI_User_function* merge_op = [](void* invec, void* inoutvec, int* len, MPI_Datatype*) {
        TopK<K, ValueType>* invec_    = static_cast<TopK<K, ValueType>*>(invec);
        TopK<K, ValueType>* inoutvec_ = static_cast<TopK<K, ValueType>*>(inoutvec);
        std::transform(invec_, invec_ + *len, inoutvec_, inoutvec_, merge<K, size_t>);
    };
    MPI_Op_create(merge_op, true, &top_k_merge_op);

    // Next, perform the actual communication using plain MPI
    TopK<K, ValueType> global_top_k;
    MPI_Reduce(&local_top_k, &global_top_k, 1, top_k_type, top_k_merge_op, 0, comm);

    // Finally, clean up the custom datatype and reduce operation.
    MPI_Op_free(&top_k_merge_op);
    MPI_Type_free(&top_k_type);

    int rank;
    MPI_Comm_rank(comm, &rank);
    return rank == 0 ? std::make_optional(std::move(global_top_k)) : std::optional<TopK<K, ValueType>>{};
}

// Top-K reduction using xxxxx-1
template <size_t K, typename ValueType>
std::optional<TopK<K, ValueType>>
xxxxx-1ing_top_k(TopK<K, ValueType> const& local_top_k, xxxxx-1ing::BasicCommunicator& comm) {
    using namespace xxxxx-1ing;

    // xxxxx-1 simplifies the callsite by:
    // - Providing a named parameter interface.
    // - Defaulting to using rank 0 as root.
    // - Automatically deducing the datatype of the elements.
    // - Automatically creating and de-creating the custom datatype and the reduce operation.
    auto result = comm.reduce(send_buf(local_top_k), op(merge<K, size_t>, ops::commutative));

    return comm.is_root() ? std::make_optional(result[0]) : std::optional<TopK<K, ValueType>>{};
}

int main(int argc, char* argv[]) {
    namespace kmp = xxxxx-1ing;

    // xxxxx-1 wraps MPI_Init and MPI_Finalize using the RAII principle.
    kmp::Environment       env(argc, argv);
    kmp::BasicCommunicator comm;

    constexpr size_t K = 3;
    TopK<K, size_t>  input;
    for (size_t idx = 0; idx < K; ++idx) {
        input[idx] = comm.rank() + idx * comm.size();
    }
    std::cout << "[R" << comm.rank() << "] local_input=" << input << std::endl;

    auto xxxxx-1ing_result = xxxxx-1ing_top_k(input, comm);
    if (comm.is_root()) {
        std::cout << "global_result_xxxxx-1ing=" << xxxxx-1ing_result.value() << std::endl;
    }

    auto mpi_result = mpi_top_k(input, MPI_COMM_WORLD);
    int  rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        std::cout << "global_result_mpi=" << mpi_result.value() << std::endl;
    }
}
