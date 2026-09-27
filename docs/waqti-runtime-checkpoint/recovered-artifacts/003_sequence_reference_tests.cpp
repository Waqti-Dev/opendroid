// SPDX-License-Identifier: Apache-2.0
#include "../transformer_layer.h"
#include "../attention_reference.h"
#include <cassert>
#include <cmath>
#include <string>
#include <vector>

int main() {
    std::string error;
    waqti::transformer::KVCache cache;
    assert(cache.initialize(2, 1, 2, &error));
    assert(cache.positions(0) == 0);
    assert(cache.append(0, {1.0f, 0.0f}, {2.0f, 3.0f}, &error));
    assert(cache.append(0, {0.0f, 1.0f}, {4.0f, 5.0f}, &error));
    assert(cache.positions(0) == 2 && cache.positions(1) == 0);
    std::vector<float> output;
    assert(waqti::attention::causal_attention({1.0f, 0.0f}, {1.0f, 0.0f}, {2.0f, 3.0f},
                                               1, 1, 1, 2, output, &error));
    assert(output.size() == 2 && std::fabs(output[0] - 2.0f) < 1e-6f && std::fabs(output[1] - 3.0f) < 1e-6f);
    assert(waqti::attention::causal_attention({1.0f, 0.0f}, cache.keys[0], cache.values[0],
                                               2, 1, 1, 2, output, &error));
    assert(output[0] > 2.0f && output[0] < 4.0f);
    assert(output[1] > 3.0f && output[1] < 5.0f);
    assert(!cache.append(0, {1.0f}, {2.0f}, &error));
    return 0;
}
