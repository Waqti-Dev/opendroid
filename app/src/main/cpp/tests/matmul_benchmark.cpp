// SPDX-License-Identifier: Apache-2.0
#include "../tensor_primitives.h"
#include <chrono>
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    const size_t n = argc > 1 ? static_cast<size_t>(std::strtoul(argv[1], nullptr, 10)) : 256;
    const size_t repeats = argc > 2 ? static_cast<size_t>(std::strtoul(argv[2], nullptr, 10)) : 10;
    waqti::tensor::Matrix a{n, n, std::vector<float>(n * n, 0.001f)};
    waqti::tensor::Matrix b{n, n, std::vector<float>(n * n, 0.002f)};
    waqti::tensor::Matrix out; std::string error;
    const auto start = std::chrono::steady_clock::now();
    for (size_t i = 0; i < repeats; ++i) if (!waqti::tensor::matmul(a, b, out, &error)) return 2;
    const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    const double gflops = (2.0 * n * n * n * repeats) / elapsed / 1e9;
    std::cout << "n=" << n << " repeats=" << repeats << " seconds=" << elapsed << " gflops=" << gflops << "\\n";
    return 0;
}
