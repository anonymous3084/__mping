# xxxxx-1: xxxxx-1 MPI next generation :rocket:

![xxxxx-1 logo](./docs/images/logo.svg)

This is xxxxx-1 [xxxxx-1ɪŋ], a (near) zero-overhead MPI wrapper for modern C++.

It covers the whole range of abstraction levels from low-level MPI calls to
convenient STL-style bindings, where most parameters are inferred from a small
subset of the full parameter set. This allows for both rapid prototyping and
fine-tuning of distributed code with predictable runtime behavior and memory
management.

Using template-metaprogramming, only code paths required for computing
parameters not provided by the user are generated at compile time, which results in (near) zero-overhead
bindings.

**:running: Quick Start:** We provide a wide range of [usage](./examples/usage) and [simple applications](./examples/applications) examples (start with [`allgatherv`](./examples/usage/allgatherv_example.cpp)).

xxxxx-1 is developed at the [xxxxx-1
Group](https://xxxxx-1/english/index.php) at xxxxx-1 xxxxx-1
Technology.

## Features :sparkles:
### Named Parameters :speech_balloon:
Using plain MPI, operations like `MPI_Allgatherv` often lead to verbose and error-prone boilerplate code:

``` c++
std::vector<T> v = ...; // Fill with data
int size;
MPI_Comm_size(comm, &size);
int n = static_cast<int>(v.size());
std::vector<int> rc(size), rd(size);
MPI_Allgather(&n, 1, MPI_INT, rc.data(), 1, MPI_INT, comm);
std::exclusive_scan(rc.begin(), rc.end(), rd.begin(), 0);
int n_glob = rc.back() + rd.back();
std::vector<T> v_glob(v_global_size);
MPI_Allgatherv(v.data(), v_size, MPI_TYPE, v_glob.data(), rc.data(), rd.data(), MPI_TYPE, comm);

```

In contrast, xxxxx-1 introduces a streamlined syntax inspired by Python's named parameters. For example, the `allgatherv` operation becomes more intuitive and concise:

```c++
std::vector<T> v = ...; // Fill with data
std::vector<T> v_glob = comm.allgatherv(send_buf(v));
```

Empowered by named parameters, xxxxx-1 allows users to name and pass parameters in arbitrary order, computing default values only for the missing ones. This not only improves readability but also streamlines the code, providing a user-friendly and efficient way of writing MPI applications.

### Controlling memory allocation :floppy_disk:
xxxxx-1's *resize policies* allow for fine-grained control over when allocation happens:

| resize policy            |                                                                         |
|--------------------------|-------------------------------------------------------------------------|
| `xxxxx-1ing::resize_to_fit` | resize the container to exactly accommodate the data                    |
| `xxxxx-1ing::no_resize`     | assume that the container has enough memory available to store the data |
| `xxxxx-1ing::grow_only`     | only resize the container if it not large enough                        |


``` c++
// easy to use with sane defaults
std::vector<int> v = comm.recv<int>(source(xxxxx-1ing::rank::any));

// flexible memory control
std::vector<int> v_out;
v_out.resize(enough_memory_to_fit);
// already_known_counts are the recv_counts that may have been computed already earlier and thus do not need to be computed again
comm.recv<int>(recv_buf<xxxxx-1ing::no_resize>(v_out), recv_count(i_know_already_know_that), source(xxxxx-1ing::rank::any));
```

### STL support :books:
- xxxxx-1 works with everything that is a `std::contiguous_range`, everywhere.
- Builtin C++ types are automatically mapped to their corresponding MPI types. 
- All internally used containers can be altered via template parameters.
### Expandability :jigsaw:
- Don't like the performance of your MPI implementation's reduce algorithm? Just override it using our plugin architecture.
- Add additional functionality to communicator objects, without altering any application code.
- Easy to integrate with existing MPI code.
- Flexible core library for a new toolbox :toolbox: of distributed datastructures and algorithms

### And much more ... :arrow_upper_right:
- Easy non-blocking communication via request pools.
- Compile time and runtime error checking (which can be completely deactivated).
- Collective hierarchical timers to speed up your evaluation workflow.
- ...

Dive into the documentation or tests to find out more ...

### (Near) zero overhead - for development and performance :chart_with_upwards_trend:
Using template-metaprogramming, xxxxx-1 only generates the code paths required for computing parameters not provided by the user. 
The following shows a complete implementation of distributed sample sort with xxxxx-1. 

```c++
void sort(MPI_Comm comm_, std::vector<T>& data, size_t seed) {
    Communicator<> comm(comm_);
    size_t const   oversampling_ratio = 16 * static_cast<size_t>(std::log2(comm.size())) + 1;
    std::vector<T> local_samples(oversampling_ratio);
    std::sample(data.begin(), data.end(), local_samples.begin(), oversampling_ratio, std::mt19937{seed});
    auto global_samples = comm.allgather(send_buf(local_samples)).extract_recv_buffer();
    std::sort(global_samples.begin(), global_samples.end());
    for (size_t i = 0; i < comm.size() - 1; i++) {
        global_samples[i] = global_samples[oversampling_ratio * (i + 1)];
    }
    global_samples.resize(num_splitters);
    std::vector<std::vector<T>> buckets(global_samples.size() + 1);
    for (auto& element: data) {
        auto const bound = std::upper_bound(global_samples.begin(), global_samples.end(), element);
        buckets[static_cast<size_t>(bound - global_samples.begin())].push_back(element);
    }
    data.clear();
    std::vector<int> scounts;
    for (auto& bucket: buckets) {
        data.insert(data.end(), bucket.begin(), bucket.end());
        scounts.push_back(static_cast<int>(bucket.size()));
    }
    data = comm.alltoallv(send_buf(data), send_counts(scounts)).extract_recv_buffer();
    std::sort(data.begin(), data.end());
}
```
It is a lot more concise than the [(verbose) plain MPI implementation](./examples/applications/sample-sort/mpi.hpp), but also introduces no additional overhead to achieve this, as can be seen the following experiment. There we compare the sorting implementation in xxxxx-1 to other MPI bindings.

![](./plot.svg)
## Platform :desktop_computer:
- intensively tested with GCC and Clang and OpenMPI
- requires a C++17 ready compiler
- easy integration into other projects using modern CMake
   
## Other MPI bindings
|                                                      | [MPI](https://www.mpi-forum.org/) | [Boost.MPI](https://www.boost.org/doc/libs/1_84_0/doc/html/mpi.html) | [RWTH MPI](https://github.com/VRGroupRWTH/mpi) | [MPL](https://github.com/rabauke/mpl) | ![xxxxx-1](./docs/images/icon.svg) |
|------------------------------------------------------|:---------------------------------:|:--------------------------------------------------------------------:|:----------------------------------------------:|:-------------------------------------:|:-----------------------------------------------:|
| STL support                                          | :x:                               | :heavy_check_mark:[^2]                                               | :heavy_check_mark:[^3]                         | :heavy_check_mark:[^2]                | :white_check_mark:                              |
| computation of defaults via additional communication | :x:                               | :x:                                                                  | :white_check_mark:                             | :x:                                   | :white_check_mark:                              |
| custom reduce operations via lambdas                 | :x:                               | :white_check_mark:                                                   | :x:                                            | :heavy_check_mark:[^4]                | :white_check_mark:                              |
| containers can be resized automatically              | :x:                               | :heavy_check_mark:[^1]                                               | :heavy_check_mark:[^3]                         | :x:                                   | :white_check_mark:                              |
| error handling                                       | :white_check_mark:                | :white_check_mark:                                                   | :white_check_mark:                             | :x:                                   | :white_check_mark:                              |
| actively maintained                                  | :white_check_mark:                | :x:                                                                  | :heavy_check_mark:                             | :white_check_mark:                    | :white_check_mark:                              |

[^1]: partial 

[^2]: only `std::vector` 

[^3]: only for send and receive buffers

[^4]: not mapped to builtin operations

## LICENSE
xxxxx-1 is released under the GNU Lesser General Public License. See [COPYING](COPYING) and [COPYING.LESSER](COPYING.LESSER) for details
