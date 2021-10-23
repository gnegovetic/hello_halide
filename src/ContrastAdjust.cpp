#include <stdio.h>
#include <assert.h>
#include <array>
#include <random>
#include <functional>
#include <algorithm>
#include <memory>

#include "contrast_adjust.h"
#include "min_max.h"
#include "HalideBuffer.h"
#include "halide_benchmark.h"
using namespace std;
using namespace Halide;


// C++ implemenation of the contrast adjustment
template<class T, size_t SIZE>
static unique_ptr<array<T, SIZE>> AdjustContrast(const array<T, SIZE>& image) {
    auto maxVal = *(max_element(image.begin(), image.end()));
    auto minVal = *(min_element(image.begin(), image.end()));
 
    auto outImage = make_unique<array<T, SIZE>>();
    auto it = outImage->begin();
    float scale = 255.0f / (float)(maxVal - minVal);

    for (auto& v : image) {
        *it = (uint8_t)((v - minVal) * scale + 0.5f);
        it++;
    }
 
    return outImage;
}
 
int main()
{
    // Create test input data
    using tImageBuffer = array<uint8_t, 1920 * 1080>;
    auto image = make_unique<tImageBuffer>();

    // Fill out image with random data
    const auto InputMax = 220;
    const auto InputMin = 55;
    default_random_engine generator;
    uniform_int_distribution<uint16_t> distribution(InputMin, InputMax);
    auto value = bind(distribution, generator);
    for (auto& v : *image) {
        v = static_cast<uint8_t>(value());
    }
 
    // Time the C++ implementation
    constexpr int samples = 1;
    constexpr int iterations = 100;
    unique_ptr<tImageBuffer> outImage = nullptr;

    double c_impl_time = Tools::benchmark(samples, iterations, [&]() {
        outImage = AdjustContrast(*image);
    });
    
    // Verify adjusted contrast
    auto maxVal = *max_element(outImage->begin(), outImage->end());
    auto minVal = *min_element(outImage->begin(), outImage->end());
    assert(maxVal == 255);
    assert(minVal == 0);
    printf("Output pixel value range: [%d:%d], time: %f s\n", minVal, maxVal, c_impl_time);

    // Adjust contrast (Halide implementation)
    Runtime::Buffer<uint8_t> input_buf(image->begin(), {1920, 1080});
    Runtime::Buffer<uint8_t> output_buf(1920, 1080);

    double halide_impl_time = Tools::benchmark(samples, iterations, [&]() {

        // first calculate min/max
        Runtime::Buffer<uint8_t> min_val(1);
        Runtime::Buffer<uint8_t> max_val(1);
        int error = min_max(input_buf, min_val, max_val);
        assert(error == 0);

        uint8_t minValHalide = *min_val.data();
        uint8_t maxValHalide = *max_val.data();
        //printf("Halide input pixel value range: [%d:%d]\n", minValHalide, maxValHalide);

        error = contrast_adjust(input_buf, minValHalide, maxValHalide, output_buf);
        assert(error == 0);
    });
 
    // Verify adjusted contrast
    maxVal = *max_element(output_buf.begin(), output_buf.end());
    minVal = *min_element(output_buf.begin(), output_buf.end());
    printf("Halide Output pixel value range: [%d:%d], time: %f s\n", minVal, maxVal, halide_impl_time);
    assert(maxVal == 255);
    assert(minVal == 0);

    // Check values
    size_t errors = 0;
    for (size_t i = 0; i < outImage->size(); i++) {
        if ((*outImage)[i] != (output_buf.begin())[i]) {
            errors++;
        }
    }
 
    if (errors > 0) {
        printf("%zu values are different.\n", errors);
        return -1;
    }
    else {
        printf("Success!\n");
        return 0;
    }
}