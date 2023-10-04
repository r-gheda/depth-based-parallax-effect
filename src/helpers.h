#pragma once
/*
This file contains useful function and definitions.
Do not ever edit this file - it will not be uploaded for evaluation.
If you want to modify any of the functions here (e.g. extend triangle test to quads),
copy the function "your_code_here.h" and give it a new name.
*/
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()

#include <cassert>
#include <chrono>
#include <cmath>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#ifdef _OPENMP
// Only if OpenMP is enabled.
#include <omp.h>
#endif

#include <framework/image.h>

/// <summary>
/// Aliases for Image classes.
/// </summary>
using ImageRGB = Image<glm::vec3>;
using ImageFloat = Image<float>;

/// <summary>
/// Structure for the XYZ image.
/// </summary>
struct ImageXYZ {
    ImageFloat x;
    ImageFloat y;
    ImageFloat z;
};

/// <summary>
/// Structure for the gradient image.
/// </summary>
struct ImageGradient {
    ImageFloat x;
    ImageFloat y;
};


/// <summary>
/// Prints helpful information about OpenMP.
/// </summary>
void printOpenMPStatus() 
{
#ifdef _OPENMP
    // https://stackoverflow.com/questions/38281448/how-to-check-the-version-of-openmp-on-windows
    std::cout << "OpenMP version " << _OPENMP << " is ENABLED with " << omp_get_max_threads() << " threads." << std::endl;
#else
    std::cout << "OpenMP is DISABLED." << std::endl;
#endif
}

/// <summary>
/// Converts gradients to RGB for visualization.
/// Red channel = dX, Green  = dY, Blue = 0.
/// </summary>
/// <param name="gradient"></param>
/// <returns></returns>
ImageRGB gradientsToRgb(const ImageGradient& gradient) {
    auto grad_rgb = ImageRGB(gradient.x.width, gradient.x.height);
    for (auto i = 0; i < grad_rgb.data.size(); i++) {
        grad_rgb.data[i] = glm::vec3(gradient.x.data[i], gradient.y.data[i], 0);
    }
    return grad_rgb;
}


/// <summary>
/// Converts float image to RGB by repeating the channel 3x.
/// </summary>
/// <param name="img"></param>
/// <returns></returns>
ImageRGB imageFloatToRgb(const ImageFloat& img) {
    auto result = ImageRGB(img.width, img.height);
    #pragma omp parallel for
    for (int i = 0; i < result.data.size(); i++) {
        result.data[i] = glm::vec3(img.data[i], img.data[i], img.data[i]);
    }
    return result;
}

/// <summary>
/// Converts RGB image to float by selecting the Red channel.
/// </summary>
/// <param name="img"></param>
/// <returns></returns>
ImageFloat imageRgbToFloat(const ImageRGB& img)
{
    auto result = ImageFloat(img.width, img.height);
#pragma omp parallel for
    for (int i = 0; i < result.data.size(); i++) {
        result.data[i] = img.data[i].x;
    }
    return result;
}

/// <summary>
/// Compute natural log of image offset by 1.
/// </summary>
/// <param name="image"></param>
/// <returns></returns>
ImageFloat lnImage(const ImageFloat& image)
{
    auto result = ImageFloat(image.width, image.height);
    for (int i = 0; i < image.data.size(); i++) {
        result.data[i] = glm::log(image.data[i] + 1.0f);
    }
    return result;
}

