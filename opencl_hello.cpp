#include <iostream>
#include <CL/opencl.hpp>

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

const char *kernelSource = R"(
    kernel void hello_world() {
        printf("Hello, World! from OpenCL\\n");
    }
)";

int main()
{

	cl::Platform default_platform = getDefaultOpenCLPlatform();
	cl::Device default_device = getDefaultOpenCLDevice(default_platform);

	cl::Context context({default_device});

	// Create a command queue for the chosen device
	cl::CommandQueue queue(context, default_device);

	// Create an OpenCL program from the kernel source
	cl::Program program(context, kernelSource);

	// Build the OpenCL program
	program.build("-cl-std=CL3.0");

	// Create an OpenCL kernel from the program
	cl::Kernel kernel(program, "hello_world");

	// Execute the OpenCL kernel
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(1), cl::NullRange);
	queue.finish();
	return 0;
}