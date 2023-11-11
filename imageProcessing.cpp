/*
Program output:
Took 0[ms]: To prepare the images
Took 53[ms]: To Set up OpenCL
Took 401[ms]: To build OpenCL program/kernal
Took 595[ms]: To fill images with random pixels
seed val1: 0
Took 35[ms]: To set OpenCL addPixelColorsKernel arguments
Took 75[ms]: To read result back from OpenCL device
Took 1427[ms]: Total time
*/

// C++ OpenCL 3.0 API Spec: https://registry.khronos.org/OpenCL/specs/3.0-unified/pdf/OpenCL_API.pdf
// C++ OpenCL 3.0 Reference guide: https://www.khronos.org/files/opencl30-reference-guide.pdf
#include <CL/opencl.hpp>
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>


using namespace std;

struct Pixel
{
    float red;
    float green;
    float blue;
};

ostream& operator<<(ostream& os, const Pixel& book) {
    return os << "P(" << book.red << ", " << book.green << ", " << book.blue << ")";
}



Pixel* createPixels(int imageSize)
{
    Pixel* image = new Pixel[imageSize];
    for (int i = 0; i < imageSize; i++)
    {
        image[i].red = (float(rand()) / float((RAND_MAX)));
        image[i].green = (float(rand()) / float((RAND_MAX)));
        image[i].blue = (float(rand()) / float((RAND_MAX)));
    }
    return image;
}




cl::Platform getDefaultOpenCLPlatform(bool verbose = false)
{
	// get all platforms (drivers)
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0)
	{
		if (verbose)
			std::cout << "No platforms found. Check OpenCL installation!" << std::endl;
		;
		exit(1);
	}
	cl::Platform default_platform = all_platforms[0];
	if (verbose)
	{
		std::cout << "All platforms:" << std::endl;
		for (size_t i = 0; i < all_platforms.size(); i++)
		{
			std::cout << "\tPlatform " << i << ": " << all_platforms[i].getInfo<CL_PLATFORM_NAME>() << std::endl;
		}
		std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
	};
	return default_platform;
}

cl::Device getDefaultOpenCLDevice(cl::Platform platform, bool verbose = false)
{
	// get default device of the default platform
	std::vector<cl::Device> all_devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	if (all_devices.size() == 0)
	{
		if (verbose)
			std::cout << "No devices found. Check OpenCL installation!" << std::endl;
		exit(1);
	}

	// use device[1] because that's a GPU; device[0] is the CPU
	// cl::Device default_device = all_devices [ 1 ];
	// std::cout << "Using device: " << default_device.getInfo < CL_DEVICE_NAME >()<< "\n" ;
	// use device[0] because that's a CPU; device[1] is the GPU (if available)
	// I don't have a GPU, so I'm using the CPU
	cl::Device default_device = all_devices[0];

	if (verbose)
	{
		std::cout << "All devices:" << std::endl;
		for (size_t i = 0; i < all_devices.size(); i++)
		{
			std::cout << "\tDevice " << i << ": " << all_devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
		}
		std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";
	}
	return default_device;
}



int main()
{

    std::ifstream file("imageProcessing.ocl");
    if (!file.is_open()) {
        std::cerr << "Failed to open kernel file" << std::endl;
        return -1;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string kernelSource = ss.str();

    auto program_start = chrono::high_resolution_clock::now();
    auto begin = chrono::high_resolution_clock::now();
    // Prepare images
    constexpr int imageSize = 4096 * 4096;
    // Pixel* image1 = createPixels(imageSize);
    // Pixel* image2 = createPixels(imageSize);
    Pixel* image1 = new Pixel[imageSize];
    Pixel* image2 = new Pixel[imageSize];
    Pixel* result = new Pixel[imageSize];
    // Prepared images
    auto end = chrono::high_resolution_clock::now();
    std::cout << "Took " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]: To prepare the images" << std::endl;


    begin = chrono::high_resolution_clock::now();
    // OpenCL setup
    cl::Platform default_platform = getDefaultOpenCLPlatform();
    cl::Device default_device = getDefaultOpenCLDevice(default_platform);
    cl::Context context({default_device});
    cl::CommandQueue queue(context, default_device);
    // OpenCL setup finished
    end = chrono::high_resolution_clock::now();
    std::cout << "Took " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]: To Set up OpenCL" << std::endl;
    

    // Create OpenCL buffer for the image
    cl::Buffer oclBufferImage1(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               sizeof(Pixel) * imageSize, image1);
    cl::Buffer oclBufferImage2(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(Pixel) * imageSize, image2);
    cl::Buffer oclBufferResult(context, CL_MEM_WRITE_ONLY,
                                 sizeof(Pixel) * imageSize);


    begin = chrono::high_resolution_clock::now();
    // Build OpenCL program and create addPixelColorsKernel
    cl::Program program(context, kernelSource);
    auto err = program.build("-cl-std=CL3.0");
    if (err != CL_SUCCESS)
	{
		std::cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
		exit(1);
	}
    cl::Kernel generateRandomPixelsKernel(program, "generateRandomPixels");
    cl::Kernel addPixelColorsKernel(program, "addPixelColors");
    // OpenCL program/kernal built
    end = chrono::high_resolution_clock::now();
    std::cout << "Took " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]: To build OpenCL program/kernal" << std::endl;


    size_t globalWorkSize = imageSize;
    begin = chrono::high_resolution_clock::now();
    // Generate random pixels for image1
    unsigned long seedImg1 = 0;
    generateRandomPixelsKernel.setArg(0, oclBufferImage1);
    generateRandomPixelsKernel.setArg(1, imageSize);
    generateRandomPixelsKernel.setArg(2, seedImg1);
    queue.enqueueNDRangeKernel(generateRandomPixelsKernel, cl::NullRange, globalWorkSize, cl::NullRange);
    queue.finish();
    // Read result back from OpenCL device
    queue.enqueueReadBuffer(oclBufferImage1, CL_TRUE, 0, sizeof(Pixel) * imageSize, image1);
    queue.finish();

    // Generate random pixels for image2
    // unsigned long seedImg2 = 0;
    generateRandomPixelsKernel.setArg(0, oclBufferImage2);
    generateRandomPixelsKernel.setArg(1, imageSize);
    generateRandomPixelsKernel.setArg(2, seedImg1);
    queue.enqueueNDRangeKernel(generateRandomPixelsKernel, cl::NullRange, globalWorkSize, cl::NullRange);
    queue.finish();
    // Read result back from OpenCL device
    queue.enqueueReadBuffer(oclBufferImage2, CL_TRUE, 0, sizeof(Pixel) * imageSize, image2);
    queue.finish();
    // Filled with random pixels
    end = chrono::high_resolution_clock::now();
    std::cout << "Took " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]: To fill images with random pixels" << std::endl;


    std::cout << "seed val1: " << seedImg1 << std::endl;
    // std::cout << "seed val2: " << seedImg2 << std::endl;

    begin = chrono::high_resolution_clock::now();
    // OpenCL addPixelColorsKernel to result
    addPixelColorsKernel.setArg(0, oclBufferImage1);
    addPixelColorsKernel.setArg(1, oclBufferImage2);
    addPixelColorsKernel.setArg(2, oclBufferResult);
    addPixelColorsKernel.setArg(3, imageSize);
    queue.enqueueNDRangeKernel(addPixelColorsKernel, cl::NullRange, globalWorkSize, cl::NullRange);
    queue.finish();
    // OpenCL addPixelColorsKernel arguments set
    end = chrono::high_resolution_clock::now();
    std::cout << "Took " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]: To set OpenCL addPixelColorsKernel arguments" << std::endl;


    begin = chrono::high_resolution_clock::now();
    // Read result back from OpenCL device
    queue.enqueueReadBuffer(oclBufferResult, CL_TRUE, 0, sizeof(Pixel) * imageSize, result);
    // Result read back from OpenCL device
    end = chrono::high_resolution_clock::now();
    std::cout << "Took " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]: To read result back from OpenCL device" << std::endl;
    

    end = chrono::high_resolution_clock::now();
    std::cout << "Took " << chrono::duration_cast<chrono::milliseconds>(end - program_start).count() << "[ms]: Total time" << std::endl;


    bool showPixels = false;
    if (showPixels){
        size_t showPixelCount = 5;
        // Show random pixels of image1
        std::cout << std::endl << "Image1: ";
        for (size_t i = 0; i < showPixelCount; i++)
        {
            std::cout << image1[i] << ", ";
        }
        std::cout << std::endl << std::endl;

        // Show random pixels of image2
        std::cout << "Image2: ";
        for (size_t i = 0; i < showPixelCount; i++)
        {
            std::cout << image2[i] << ", ";
        }
        std::cout << std::endl << std::endl;


        // Show random pixels of result
        std::cout << "Result: ";
        for (size_t i = 0; i < showPixelCount; i++)
        {
            std::cout << result[i] << ", ";
        }
        std::cout << std::endl << std::endl;
    }

    return 0;
}
