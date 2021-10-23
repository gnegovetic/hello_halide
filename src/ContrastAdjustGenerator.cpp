#include "Halide.h"
using namespace Halide;

class MinMaxGenerator : public Halide::Generator<MinMaxGenerator> {
public:

    Input<Buffer<uint8_t>> input{"input", 2};
    Output<Buffer<uint8_t>> min_val{"min_val", 1};
    Output<Buffer<uint8_t>> max_val{"max_val", 1};

    Var x, y, c;

    void generate() {

        RDom r(input);

        min_val(c) = minimum(input(r.x, r.y));
        max_val(c) = maximum(input(r.x, r.y));
    }

    void schedule() {
        if (auto_schedule) {
            input.set_estimates({{0,1920},{0,1080}});
            min_val.set_estimates({{0,1}});
            max_val.set_estimates({{0,1}});
        }
    }
};

class ContrastAdjustGenerator : public Halide::Generator<ContrastAdjustGenerator> {
public:

    Input<Buffer<uint8_t>> input{"input", 2};
    Input<uint8_t> min_val{"min_val"};
    Input<uint8_t> max_val{"max_val"};

    Output<Buffer<uint8_t>> output{"output", 2};

    Var x, y;

    void generate() {

        Expr scale = 255.0f / (max_val - min_val);
        
        Expr value = input(x, y);
        Expr scaled = (cast<float>(value) - min_val) * scale;

        output(x, y) = cast<uint8_t>(scaled + 0.5f);
    }

    void schedule() {
        if (auto_schedule) {
            input.set_estimates({{0,1920},{0,1080}});
            min_val.set_estimate(0);
            max_val.set_estimate(255);
            output.set_estimates({{0,1920},{0,1080}});
        }
        else {
            output.vectorize(x, 16).parallel(y);
        }
    }
};

HALIDE_REGISTER_GENERATOR(MinMaxGenerator, min_max);
HALIDE_REGISTER_GENERATOR(ContrastAdjustGenerator, contrast_adjust);
