#pragma once
#include "helpers.h"

#include <cmath>
#include <map>
#include <tuple>
#include <vector>

#include <iostream>

template<typename T>
int getImageOffset(const Image<T>& image, int x, int y)
{
    return x + (y * image.width);
}

int velocity(float depth)
{
    return (1 - depth)*4;
}

float gauss(const float x, const float sigma = 1.0f, const float mu = 0.0f)
{
    auto exponent = (x - mu) / sigma;
    return std::exp(-0.5f * exponent * exponent);
}

ImageWithMask forwardWarpImage(const ImageRGB& src_image, const ImageFloat& src_depth, const ImageFloat& disparity, const float warp_factor)
{
    // The dimensions of src image, src depth and disparity maps all match.
    assert(src_image.width == disparity.width && src_image.height == disparity.height);
    assert(src_image.width == disparity.width && src_depth.height == src_depth.height);
    
    // Create a new image and depth map for the output.
    auto dst_image = ImageRGB(src_image.width, src_image.height);
    auto dst_mask = ImageFloat(src_depth.width, src_depth.height);
    // Fill the destination depth mask map with zero.
    std::fill(dst_mask.data.begin(), dst_mask.data.end(), 0.0f);
    auto dst_depth = ImageFloat(src_depth.width, src_depth.height);
    // Fill the destination depth map with a very large number.
    std::fill(dst_depth.data.begin(), dst_depth.data.end(), std::numeric_limits<float>::max());
    
    #pragma omp parallel for shared(src_image, dst_image, dst_depth, dst_mask) collapse(2)
    for (int x = 0; x < src_image.width; ++x)
    {
        for (int y = 0; y < src_image.height; ++y)
        {
            int x_prime = (int) (x + disparity.data[getImageOffset(disparity, x, y)] * warp_factor + 0.5f);
            int y_prime = y;

            if (x_prime < 0 || x_prime >= src_image.width || y_prime < 0 || y_prime >= src_image.height)
            {
                continue;
            }

            float depth = src_depth.data[getImageOffset(src_depth, x, y)];
            float depth_prime = dst_depth.data[getImageOffset(src_depth, x_prime, y_prime)];

            if (depth_prime <= depth)
            {
                continue;
            }
            dst_image.data[getImageOffset(dst_image, x_prime, y_prime)] = src_image.data[getImageOffset(src_image, x, y)];
            dst_depth.data[getImageOffset(dst_depth, x_prime, y_prime)] = depth;
            dst_mask.data[getImageOffset(dst_mask, x_prime, y_prime)] = 1.0f;
        }
    }


    // Return the warped image.
    return ImageWithMask(dst_image, dst_mask);

}


ImageRGB inpaintHoles(const ImageWithMask& img, const int size)
{
    assert(size % 2 == 1);
    const float sigma = (size - 1) / 2 / 3.2f;
    auto result = ImageRGB(img.image);

    for (int x = 0; x < result.width; ++x)
    {
        for (int y = 0; y < result.height; ++y)
        {
            if (img.mask.data[getImageOffset(img.mask, x, y)] >= 0.5f)
            {
                result.data[getImageOffset(result, x, y)] = img.image.data[getImageOffset(img.image, x, y)];
                continue;
            }
            float res_r = 0;
            float res_g = 0;
            float res_b = 0;
            float weight_sum = 0;
            for (int s_x = -size / 2; s_x <= size / 2; s_x++)
            {
                for (int s_y = -size / 2; s_y <= size / 2; s_y++)
                {
                    int curr_x = x + s_x;
                    int curr_y = y + s_y;
                    if (curr_x < 0 || curr_x >= result.width || curr_y < 0 || curr_y >= result.height || img.mask.data[getImageOffset(img.mask, curr_x, curr_y)] < 0.5f)
                    {
                        continue;
                    }
                    float w_i = gauss(glm::distance(glm::vec2(x, y), glm::vec2(curr_x, curr_y)), sigma);
                    res_r += img.image.data[getImageOffset(img.image, curr_x, curr_y)].r * w_i;
                    res_g += img.image.data[getImageOffset(img.image, curr_x, curr_y)].g * w_i;
                    res_b += img.image.data[getImageOffset(img.image, curr_x, curr_y)].b * w_i;
                    weight_sum += w_i;
                }
            }
            if (weight_sum > 0)
            {
                result.data[getImageOffset(result, x, y)].r = res_r / weight_sum;
                result.data[getImageOffset(result, x, y)].g = res_g / weight_sum;
                result.data[getImageOffset(result, x, y)].b = res_b / weight_sum;
            }
        }
    }

    return result;
}

ImageFloat normalizedDepthToDisparity(
    const ImageFloat& depth, const float iod_mm,
    const float px_size_mm, const float screen_distance_mm,
    const float near_plane_mm, const float far_plane_mm)
{
    auto px_disparity = ImageFloat(depth.width, depth.height);

    for (int x = 0; x < depth.width; ++x)
    {
        for (int y = 0; y < depth.height; ++y)
        {            
            float z = (depth.data[getImageOffset(depth, x, y)]) * (far_plane_mm - near_plane_mm)  - (far_plane_mm - near_plane_mm) / 2;
            px_disparity.data[getImageOffset(px_disparity, x, y)] = (iod_mm / px_size_mm) * (z / (screen_distance_mm + z));
        }
    }

    return px_disparity; // returns disparity measured in pixels
}