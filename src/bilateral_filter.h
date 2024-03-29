#pragma once
#include "helpers.h"

#include <cmath>
#include <map>

const float DEPTH_SIGMA = 0.1;
const float SPACE_SIGMA = 2.0;

template<typename T>
int getImageOffset(const Image<T>& image, int x, int y)
{
    return x + (y * image.width);
}

std::vector<float> getGaussianKernel(float aperture_size, float sigma = SPACE_SIGMA)
{
    std::vector<float> kernel;
    int kernel_size = 2 * aperture_size + 1;
    kernel.reserve(kernel_size * kernel_size);

    float sum = 0.0f;
    for (int x = -aperture_size; x <= aperture_size; ++x)
    {
        for (int y = -aperture_size; y <= aperture_size; ++y)
        {
            float value = exp(-(x * x + y * y) / (2 * sigma * sigma));
            kernel.push_back(value);
            sum += value;
        }
    }

    for (int i = 0; i < kernel.size(); ++i)
    {
        kernel[i] /= sum;
    }

    return kernel;
}

float gaussian_kernel(float mean, float x, float sigma=DEPTH_SIGMA)
{
    return exp(- (x-mean) * (x-mean) / (2 * sigma * sigma));
}

float gaussian_kernel_2d(float x, float y, float sigma)
{
    return exp(-(x * x + y * y) / (2 * sigma * sigma));
}

ImageRGB cross_bilateral_filter(const ImageRGB& input_rgb_image, const ImageFloat& depth_map, int focus_x, int focus_y, int aperture_size, float tolerance = 0.0f)
{
    auto output_image = ImageRGB(input_rgb_image.width, input_rgb_image.height);

    // compute spatial kernel once (it will be the same for every pixel)
    auto spatial_kernel = getGaussianKernel(aperture_size);

    #pragma omp parallel for shared(output_image)
    for (int xy = 0; xy < output_image.width*output_image.height; ++xy)
    {
        int x = xy % output_image.width;
        int y = xy / output_image.width;  

        // initialize normalization factor as the wieght of the pixel itself (1)
        float normalization_factor = gaussian_kernel(depth_map.data[getImageOffset(depth_map, focus_x, focus_y)], depth_map.data[getImageOffset(depth_map, x, y)]);
        // insert it in the output image
        output_image.data[getImageOffset(output_image, x, y)] = normalization_factor * input_rgb_image.data[getImageOffset(input_rgb_image, x, y)];

        #pragma omp parallel for shared(output_image)
        for (int x_a = -aperture_size; x_a < aperture_size; ++x_a)
        {
            #pragma omp parallel for shared(output_image)
            for (int y_a = -aperture_size; y_a < aperture_size; ++y_a)
            {
                int x_i = x + x_a;
                int y_i = y + y_a;

                // check if the pixel is out of bounds
                if (x_i < 0 || x_i >= output_image.width || y_i < 0 || y_i >= output_image.height)
                {
                    continue;
                }
                // get the spatial weight from the kernel
                float spatial_weight = spatial_kernel[(x_a + aperture_size) * (2 * aperture_size + 1) + (y_a + aperture_size)];
                // get the depth weight
                float depth_weight = pow(1-gaussian_kernel(depth_map.data[getImageOffset(depth_map, focus_x, focus_y)], depth_map.data[getImageOffset(depth_map, x_i, y_i)]), 2);
                // compute the weight and add it to the normalization factor
                float weight = spatial_weight * depth_weight;
                normalization_factor += weight;

                // add the weighted pixel to the output image
                output_image.data[getImageOffset(output_image, x, y)].r += weight * input_rgb_image.data[getImageOffset(input_rgb_image, x_i, y_i)].r;
                output_image.data[getImageOffset(output_image, x, y)].g += weight * input_rgb_image.data[getImageOffset(input_rgb_image, x_i, y_i)].g;
                output_image.data[getImageOffset(output_image, x, y)].b += weight * input_rgb_image.data[getImageOffset(input_rgb_image, x_i, y_i)].b;
            }

        }
        // normalize the output pixel
        output_image.data[getImageOffset(output_image, x, y)].r /= normalization_factor;
        output_image.data[getImageOffset(output_image, x, y)].g /= normalization_factor;
        output_image.data[getImageOffset(output_image, x, y)].b /= normalization_factor;
    
    }

    return output_image;
}
