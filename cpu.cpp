#include <iostream>
#include <thread>
#include <vector>

void worker(int thread_id)
{
	while (true)
	{
		std::cout << "Thread " << thread_id << " is working" << std::endl;
	}
}

int main()
{
	// Get the number of logical processors.
	int num_cores = std::thread::hardware_concurrency();
	// Print the number of cores.
	std::cout << "This machine supports concurrency with " << num_cores
			  << " cores available" << std::endl;

	// Create a vector to store the threads.
	std::vector<std::thread> threads;

	// Create a thread for each core.
	for (int i = 1; i <= num_cores; i++)
	{
		threads.push_back(std::thread(worker, i));
	}

	// Wait for all threads to finish.
	for (auto &thread : threads)
	{
		thread.join();
	}
	return 0;
}
