# hello_halide

## Objectives
The objective of this project is to demonstrate a realistic production-ready implementation of an image processing algorithm using Halide. This includes:
- Use state-of-the-art auto-scheduler for optimal performance
- Create fully compiled code with no runtime dependencies 
- Check Halide implementation correctness against baseline C++ implementation
- Compare performance between Halide and baseline C++ implementation

## Setup
I used a Mac. The code should be easy to port to any Linux by adjusting the cmake file. 
- Install XCode command-line tools (for clang/llvm)
- Install Halide and cmake: ``` brew install halide cmake ```

## Build and run
``` 
mkdir build && cd build
cmake ..
make
./contrastAdjustTest 
```

## Example output
```
./contrastAdjustTest 
Output pixel value range: [0:255], time: 0.014047 s
Halide Output pixel value range: [0:255], time: 0.000385 s
```
In this case, we achieved 36x speedup over C++ code

## Algorithm
The algorithm is a basic contrast adjustment of a grayscale image. For a given image, the pixel values are linearly scaled to maximize the dynamic range of [0-255] pixel values. 

## Code organization
Halide code is split into two generated functions: one to calculate the minimum and maximum of the pixel values, and another to apply the scaling. 

## Build process
We first compile the Halide code into a generator. Then we run the generator to create a static library for a specific platform target. Finally, we compile a test app that links in the Halide generated binary. This test app we can run to benchmark performance and check correctness. 

## Checking runtime dependencies
``` otool -L contrastAdjustTest ```



