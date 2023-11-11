/*
Take a look at the following program. It specifies pixel data as red, green and blue components,
creates two images from them and uses addPixelColors function to produce a third image.
Modify this program to execute in parallel for the performance gain.
Try to analyze the code to identify the concurrent parts.
After each modification, measure and note the gains in your source code.

Program output:
Took 4396[ms]: To prepare the images
Took 1407[ms]: To add pixel colors
Execution time: 5803[ms]
*/


#include <iostream>
#include <chrono>



using namespace std;

struct Pixel
{
    float red;
    float green;
    float blue;
};

ostream& operator<<(ostream& os, const Pixel& pixel) {
	int red = pixel.red*255;
	int green = pixel.green*255;
	int blue = pixel.blue*255;
    return os << "P(" << red << ", " << green << ", " << blue << ")";
}


void addPixelColors(const Pixel* image1, const Pixel* image2, Pixel* result, int imageSize)
{
    for (int i = 0; i < imageSize; i++)
    {
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
    auto program_start = chrono::high_resolution_clock::now();
	auto begin = chrono::high_resolution_clock::now();
	// Prepare images
    constexpr int imageSize = 4096 * 4096;
    Pixel* image1 = createPixels(imageSize);
    Pixel* image2 = createPixels(imageSize);
    Pixel* result = new Pixel[imageSize];
	// Prepared images
	auto end = chrono::high_resolution_clock::now();
	std::cout << "Took " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]: To prepare the images" << std::endl;


	begin = chrono::high_resolution_clock::now();
	// Add pixel colors
    addPixelColors(image1, image2, result, imageSize);
	// Added pixel colors
    end = chrono::high_resolution_clock::now();
	std::cout << "Took " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]: To add pixel colors" << std::endl;


	end = chrono::high_resolution_clock::now();
    cout << "Execution time: " << chrono::duration_cast<chrono::milliseconds>(end - program_start).count() << "[ms]\n";



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

    delete[] result;
    delete[] image2;
    delete[] image1;
}
