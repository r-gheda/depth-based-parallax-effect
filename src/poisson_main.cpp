#include "poisson.h"
#include <fstream>
#include <map>
#include <vector>
#include <string.h>

#include "omp.h"
#include <chrono>

int main(int argc, char** argv)
{
    printOpenMPStatus();

    if (argc != 8)
    {
        std::cerr << "Usage: " << argv[0] << " <input_image> <src_rgb_image> <output_image> <scribbles> <method> <n_of_itertions>" << std::endl;
    }
    auto input_image = ImageFloat(argv[1]);
    auto input_greyscale = ImageFloat(argv[2]);

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
    }

    auto start = std::chrono::steady_clock::now();
    
    if (strcmp(argv[5], "poisson") == 0){
    // Solve Poisson equation
        std::cout << "Solving Poisson equation..." << std::endl;
        auto solved_luminance = solvePoisson(input_greyscale, scribbles, lookup, std::stoi(argv[6]));
        solved_luminance.writeToFile(argv[3]);
    } else
    {
        std::cout << "Solving Anisotropic equation..." << std::endl;
        auto solved_luminance = solveAnisotropic(input_image, input_greyscale, scribbles, lookup, std::stoi(argv[6]), std::stoi(argv[7]));
        std::cout << "done stuff";
        solved_luminance.writeToFile(argv[3]);
    }

    auto end = std::chrono::steady_clock::now();
    std::cout << "Elapsed time in minutes, seconds and milliseconds: "
        << std::chrono::duration_cast<std::chrono::minutes>(end - start).count() << "m "
        << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() - (std::chrono::duration_cast<std::chrono::minutes>(end - start).count()*60)<< "s "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() - (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() - (std::chrono::duration_cast<std::chrono::minutes>(end - start).count()*60))*1000 << "ms\n";
    return 0;
}
