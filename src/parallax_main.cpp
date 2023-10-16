#include "parallax.h"
#include <vector>
#include "omp.h"
#include <filesystem>
#include <iostream>

// namespace fs = std::filesystem;

int main(int argc, char** argv)
{
    printOpenMPStatus();

    if (argc < 3 || argc > 5)
    {
        std::cerr << "Usage: " << argv[0] << " <src_image> <depth_map> <optional: n_of_frames> <optional: epsilon>" << std::endl;
        return 1;
    }
    std::cout << "ok" << std::endl;
    auto input_image = ImageRGB(argv[1]);
    auto input_depth = ImageFloat(argv[2]);
    std::cout << "images opened" << std::endl;
    std::vector<ImageRGB> frames;
    generate_parallax_frames(frames, input_image, input_depth, argc >= 4 ? std::stoi(argv[3]) : 30, argc == 5 ? std::stof(argv[4]) : 1.0 / 255);
    
    std::cout << "saving" << std::endl;
    int it = 0;
    for (auto frame : frames)
    {
        // std::cout << "iter: " << it << std::endl;
        frame.writeToFile("../frames/output_" + std::to_string(it) + ".png");
        it++;
    }
    std::cout << "saved" << std::endl;

    return 0;
}