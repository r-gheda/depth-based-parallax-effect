#include "your_code_here.h"
#include <fstream>
#include <map>
#include <vector>
#include <string.h>

static const std::filesystem::path dataDirPath { DATA_DIR };
static const std::filesystem::path outDirPath { OUTPUT_DIR };
static const std::filesystem::path outFileName { "out.jpg" };

int main(int argc, char** argv)
{
    std::chrono::steady_clock::time_point time_start, time_end;
    // printOpenMPStatus();
    std::cout << "done stuff";

    if (argc != 7)
    {
        std::cerr << "Usage: " << argv[0] << " <input_image> <src_rgb_image> <output_image> <scribbles> <method> <n_of_itertions>" << std::endl;
    }
    auto input_image = ImageFloat(argv[1]);
    auto input_rgb_image = ImageRGB(argv[2]);

    // Load scriblles from file
    std::ifstream scribles_file(argv[4]);
    std::vector<std::vector<bool>> lookup;
    for (int i = 0; i < input_image.width; i++) {
        std::vector<bool> col;
        for (int j = 0; j < input_image.height; j++) {
            col.push_back(false);
        }
        lookup.push_back(col);
    }

    int x, y;
    float val;
    std::map<int, float> scribbles;
    while (scribles_file >> x >> y >> val) {
        scribbles[getImageOffset(input_image, y, x)] = val / 255.0f;
        lookup[y][x] = true;
        // std::cout << getImageOffset(input_image, y, x) << " " << scribbles[getImageOffset(input_image, y, x)] << std::endl;
    }
    
    if (strcmp(argv[5], "poisson") == 0){
    // Solve Poisson equation
        std::cout << "Solving Poisson equation..." << std::endl;
        auto solved_luminance = solvePoisson(input_image, scribbles, lookup, std::stoi(argv[6]));
        solved_luminance.writeToFile(outDirPath / argv[3]);
    } else
    {
        std::cout << "Solving Anisotropic equation..." << std::endl;
        auto solved_luminance = solveAnisotropic(input_image, input_rgb_image, scribbles, lookup, std::stoi(argv[6]), std::stoi(argv[7]));
        std::cout << "done stuff";
        solved_luminance.writeToFile(outDirPath / argv[3]);
    }
    return 0;
}
