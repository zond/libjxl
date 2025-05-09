#include "lib/jxl/modular/encoding/context_predict.h"
#include <iostream>

using namespace jxl;
using namespace jxl::weighted;

struct RNG {
    uint64_t out = 1;
    uint64_t next() {
        out = out * 48271 % 0x7fffffff; 
        return out;
    }
};

void step(RNG &rng, State &state, size_t xsize, size_t ysize) {
    size_t x = rng.next() % xsize;
    size_t y = rng.next() % ysize;
    std::vector<PropertyVal> properties(1);
    auto pred = state.Predict<true>(x, y, xsize, rng.next() % 256, rng.next() % 256, rng.next() % 256, rng.next() % 256, rng.next() % 256, &properties, 0);
    std::cerr << "pred = (" << pred << "i64, " << properties[0] << "i32)" << std::endl;
    state.UpdateErrors(rng.next() % 256, x, y, xsize);
    std::cerr << "state = WeightedPredictorState{prediction:[";
    for (int i = 0; i < kNumPredictors; i++) {
        std::cerr << state.prediction[i] << "i64,";
    }
    std::cerr << "], pred:" << state.pred << ", pred_errors:[";
    for (int i = 0; i < kNumPredictors; i++) {
        std::cerr << "vec![";
        for (int j = 0; j < state.pred_errors[i].size(); j++) {
            std::cerr << state.pred_errors[i][j] << "u32,";
        }
        std::cerr << "],\n";
    }
    std::cerr << ", error:vec![";
    for (int i = 0; i < state.error.size(); i++) {
        std::cerr << state.error[i] << "i64,";
    }
    std::cerr << "], header: header};" << std::endl;
}

int main() {
    Header header;
    header.all_default = true;
    RNG rng;
    header.p1C = ((uint32_t)rng.next()) % 32;
    header.p2C = ((uint32_t)rng.next()) % 32;
    header.p3Ca = ((uint32_t)rng.next()) % 32;
    header.p3Cb = ((uint32_t)rng.next()) % 32;
    header.p3Cc = ((uint32_t)rng.next()) % 32;
    header.p3Cd = ((uint32_t)rng.next()) % 32;
    header.p3Ce = ((uint32_t)rng.next()) % 32;
    uint32_t w[] = {uint32_t(rng.next()) % 16, uint32_t(rng.next()) % 16, uint32_t(rng.next()) % 16, uint32_t(rng.next()) % 16};
    memcpy(header.w, w, sizeof(header.w));
    size_t xsize = 8;
    size_t ysize = 8;
    auto state = State(header, xsize, ysize);
    step(rng, state, xsize, ysize);
    step(rng, state, xsize, ysize);
    step(rng, state, xsize, ysize);
    step(rng, state, xsize, ysize);
}