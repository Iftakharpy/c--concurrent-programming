#include <CL/opencl.hpp>
#include <iostream>
#include <chrono>

// C++ OpenCL API Spec: https://registry.khronos.org/OpenCL/specs/3.0-unified/pdf/OpenCL_API.pdf


using namespace std;

struct Pixel
{
    float red;
    float green;
    float blue;
};

const char* kernelSource = R"(
    kernel void addPixelColors(__global const float* image1,
                               __global const float* image2,
                               __global float* result,
                               int imageSize)
    {
        int i = get_global_id(0);
        
        result[i * 3] = fmin(image1[i * 3] + image2[i * 3], 1.0f);
        result[i * 3 + 1] = fmin(image1[i * 3 + 1] + image2[i * 3 + 1], 1.0f);
        result[i * 3 + 2] = fmin(image1[i * 3 + 2] + image2[i * 3 + 2], 1.0f);
    }
)";

void addPixelColorsOpenCL(const Pixel* image1, const Pixel* image2, Pixel* result, int imageSize)
{
    // Get available OpenCL platforms
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
        std::cerr << "No OpenCL platforms found." << std::endl;
        return;
    }

    // Choose the first platform
    cl::Platform platform = platforms.front();

    // Get available OpenCL devices for the chosen platform
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

    if (devices.empty()) {
        std::cerr << "No OpenCL devices found." << std::endl;
        return;
    }

    // Choose the first device
    cl::Device device = devices.front();

    // Create an OpenCL context
    cl::Context context(device);

    // Create a command queue for the chosen device
    cl::CommandQueue queue(context, device);

    // Create an OpenCL program from the kernel source
    cl::Program program(context, kernelSource);

    // Build the OpenCL program
    program.build("-cl-std=CL3.0");

    // Create OpenCL buffers for input and output data
    cl::Buffer bufferImage1(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Pixel) * imageSize, (void*)image1);
    cl::Buffer bufferImage2(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Pixel) * imageSize, (void*)image2);
    cl::Buffer bufferResult(context, CL_MEM_WRITE_ONLY, sizeof(Pixel) * imageSize);

    // Create an OpenCL kernel from the program
    cl::Kernel kernel(program, "addPixelColors");

    // Set kernel arguments
    kernel.setArg(0, bufferImage1);
    kernel.setArg(1, bufferImage2);
    kernel.setArg(2, bufferResult);
    kernel.setArg(3, imageSize);

    // Execute the OpenCL kernel
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(imageSize), cl::NullRange);
    queue.finish();

    // Read the result back to the host
    queue.enqueueReadBuffer(bufferResult, CL_TRUE, 0, sizeof(Pixel) * imageSize, result);
}

Pixel* createPixels(int imageSize)
{
    Pixel* image = new Pixel[imageSize];
    for (int i = 0; i < imageSize; i++)
    {
        image[i].red = (float(rand())/float((RAND_MAX)));
        image[i].green = (float(rand())/float((RAND_MAX)));
        image[i].blue = (float(rand())/float((RAND_MAX)));
    }
    return image;
}

int main()
{
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    constexpr int imageSize = 4096 * 4096;
    Pixel* image1 = createPixels(imageSize);
    Pixel* image2 = createPixels(imageSize);
    Pixel* result = new Pixel[imageSize];

    addPixelColorsOpenCL(image1, image2, result, imageSize);

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    cout << "Execution time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]\n";

    delete[] result;
    delete[] image2;
    delete[] image1;
}
