#include <iostream>
#include "bilateral_filter.h"

int main (int argc, char** argv)
{
    if (argc != 7)
    {
        std::cerr << "Usage: " << argv[0] << " <input_image> <depth_map> <output_image> <focus_x> <focus_y> <aperture_size>" << std::endl;
    }

    std::cout << "Reading input images..." << std::endl;

    auto input_image = ImageRGB(argv[1]);
    auto depth_map = ImageFloat(argv[2]);

    std::cout << "Reading input parameters..." << std::endl;

    int focus_x = std::stoi(argv[4]);
    int focus_y = std::stoi(argv[5]);
    int aperture_size = std::stoi(argv[6]);

    std::cout << "Applying bilateral filter..." << std::endl;

    auto output_image = cross_bilateral_filter(input_image, depth_map, focus_x, focus_y, aperture_size);
    std::cout << "Writing output image..." << std::endl;
    output_image.writeToFile(argv[3]);

    return 0;
}