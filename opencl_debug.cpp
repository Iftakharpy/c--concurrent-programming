#include <iostream>
#include <CL/opencl.hpp>

cl::Platform getDefaultOpenCLPlatform(bool verbose = true)
{
	// get all platforms (drivers)
	std::vector<cl::Platform> all_platforms;
	cl::Platform::get(&all_platforms);
	if (all_platforms.size() == 0)
	{
		if (verbose) std::cout << "No platforms found. Check OpenCL installation!" << std::endl;;
		exit(1);
	}
	cl::Platform default_platform = all_platforms[0];
	if (verbose) {
		std::cout << "All platforms:" << std::endl;
		for (size_t i = 0; i < all_platforms.size(); i++)
		{
			std::cout << "\tPlatform " << i << ": " << all_platforms[i].getInfo<CL_PLATFORM_NAME>() << std::endl;
		}
		std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
	};
	return default_platform;
}

cl::Device getDefaultOpenCLDevice(cl::Platform platform, bool verbose = true)
{
	// get default device of the default platform
	std::vector<cl::Device> all_devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
	if (all_devices.size() == 0)
	{
		if (verbose) std::cout << " No devices found. Check OpenCL installation!" << std::endl;
		exit(1);
	}
	
	// use device[1] because that's a GPU; device[0] is the CPU
	// cl::Device default_device = all_devices [ 1 ];
	// std::cout << "Using device: " << default_device.getInfo < CL_DEVICE_NAME >()<< "\n" ;
	// use device[0] because that's a CPU; device[1] is the GPU (if available)
	// I don't have a GPU, so I'm using the CPU
	cl::Device default_device = all_devices[0];

	if (verbose){
		std::cout << "All devices:" << std::endl;
		for (size_t i = 0; i < all_devices.size(); i++)
		{
			std::cout << "\tDevice " << i << ": " << all_devices[i].getInfo<CL_DEVICE_NAME>() << std::endl;
		}
		std::cout << "Using device: " << default_device.getInfo<CL_DEVICE_NAME>() << "\n";
	}
	return default_device;
}


int main(){
	cl::Platform default_platform = getDefaultOpenCLPlatform();
	cl::Device default_device = getDefaultOpenCLDevice(default_platform);
	std::cout << "Looks like OpenCL installation is ok." << std::endl;
	return 0;
};