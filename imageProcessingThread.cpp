/*
Program output:
Execution time using 1 threads: 4988[ms]
Execution time using 2 threads: 29232[ms]
Execution time using 4 threads: 20293[ms]
Execution time using 8 threads: 35789[ms]
*/

#include <iostream>
#include <chrono>
#include <thread>

using namespace std;


struct Pixel
{
    float red;
    float green;
    float blue;
};


void work_on_pixels(int start, int end, Pixel* image1, Pixel* image2, Pixel* result) {
	for (int i = start; i < end; i++)
    {
		// Set the first image to random colors
        image1[i].red = (float(rand()) / float((RAND_MAX)));
        image1[i].green = (float(rand()) / float((RAND_MAX)));
        image1[i].blue = (float(rand()) / float((RAND_MAX)));
		// Set the second image to random colors
		image2[i].red = (float(rand()) / float((RAND_MAX)));
        image2[i].green = (float(rand()) / float((RAND_MAX)));
        image2[i].blue = (float(rand()) / float((RAND_MAX)));
		
		// Add the colors of the two images together
		result[i].red = image1[i].red + image2[i].red;
		if (result[i].red > 1.0f)
		{
			result[i].red = 1.0f;
		}
		result[i].green = image1[i].green + image2[i].green;
		if (result[i].green > 1.0f)
		{
			result[i].green = 1.0f;
		}
		result[i].blue = image1[i].blue + image2[i].blue;
		if (result[i].blue > 1.0f)
		{
			result[i].blue = 1.0f;
		}
    }
}

int main()
{
    auto begin = chrono::high_resolution_clock::now();

	int imageSize = 4096 * 4096;
	Pixel* image1 = new Pixel[imageSize];
	Pixel* image2 = new Pixel[imageSize];
	Pixel* result = new Pixel[imageSize];

	int num_threads = 8;
	thread threads[num_threads];

    for (int i = 0; i < num_threads; i++)
    {
        // Split sections of the image to different threads
        threads[i] = thread(work_on_pixels, i * (imageSize / num_threads), (i + 1) * (imageSize / num_threads), image1, image2, result);
    }

    for (int i = 0; i < num_threads; i++)
    {
        threads[i].join();
    }

    auto end = chrono::high_resolution_clock::now();

    cout << "Execution time using "<<num_threads<<" threads: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]\n";

    delete[] result;
    delete[] image2;
    delete[] image1;
}
