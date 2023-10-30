#include "parallax.h"
#include <vector>
#include "omp.h"
#include <filesystem>
#include <iostream>

int main(int argc, char** argv)
{
    printOpenMPStatus();

    if (argc < 3 || argc > 5)
    {
        std::cerr << "Usage: " << argv[0] << " <src_image> <depth_map> <optional: n_of_frames> <optional: warp_factor>" << std::endl;
        return 1;
    }

    float warp_factor = 0.05;
    int frames = 10;
    if ( argc  >= 4 )
    {
        frames = std::stoi(argv[3]);
    }
    if ( argc == 5 )
    {
        warp_factor = std::stof(argv[4]);
    }

    auto image = ImageRGB(argv[1]);
    auto depth_map = ImageFloat(argv[2]);
    auto target_disparity = normalizedDepthToDisparity(
        depth_map,
        64.0f,
        0.25f,
        590.f,
        550.f,
        670.f
    );  
    
    std::string out_directory = "outputs/frames/";
    if (!std::filesystem::exists(out_directory))
    {
        std::filesystem::create_directory(out_directory);
    }

    std::string out_file = "";

    for (int i = - frames / 2; i < frames / 2; ++i)
    {
        // Forward warp the image
        auto img_forward = forwardWarpImage(image, depth_map, target_disparity, i*warp_factor);
        // Inpaint the holes in the forward warping image.
        ImageRGB dst_image = inpaintHoles(img_forward, 19);

        // Write the image to file.
        out_file = "";
        out_file.append(out_directory).append(std::to_string(i + (frames / 2)).append(".png"));
        dst_image.writeToFile(out_file, 1.0f, 0.0f);
    }
    return 0;
}
