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

// int compute_nxt(int pxl, int center, float depth){
//     if (pxl == center)
//     {
//         return pxl;
//     }

//     int diff = velocity(depth);

//     if (pxl > center)
//     {
//         return pxl + diff;
//     }

//     return pxl - diff;
// }

// float interpolate(float hi, float lo, float le, float ri)
// {
//     float div = 4.0;
//     if (hi < 0)
//     {
//         hi = 0;
//         div -= 1;
//     }
//     if (lo < 0)
//     {
//         lo = 0;
//         div -= 1;
//     }
//     if (le < 0)
//     {
//         le = 0;
//         div -= 1;
//     }
//     if (ri < 0)
//     {
//         ri = 0;
//         div -= 1;
//     }
//     if (div < 1)
//     {
//         return -1;
//     }
//     return (hi + lo + le + ri) / div;
// }

// auto interpolate(const std::vector<std::tuple<float, std::tuple<float, float, float>>> stretched_coords, int x, int y)
// {
//     for (auto el : stretched_coords)
//     {
//         if (std::get<0>(el) == x)
//         {
//             return std::get<1>(el);
//         }
//     }
// }

// void generate_parallax_frames(std::vector<ImageRGB>& output_frames, const ImageRGB& src_img, const ImageFloat& depth_map, int frames = 30, float epsilon = 1.0 / 255)
// {
//     output_frames.push_back(src_img);

//     const int X = src_img.width;
//     const int Y = src_img.height;

//     int x_center = X / 2;
//     int y_center = Y / 2;

//     auto curr_image = ImageRGB(X, Y);
//     auto prev_image = ImageRGB(X, Y);
//     auto curr_depth = ImageFloat(X, Y);
//     auto prev_depth = ImageFloat(X, Y);

//     // prevs = inputs
//     #pragma omp parallel for shared(curr_image, curr_depth)
//     for (int x = 0; x < X; ++x)
//     {
//         #pragma omp parallel for shared(curr_image, curr_depth)
//         for (int y = 0; y < Y; ++y)
//         {
//             prev_image.data[getImageOffset(curr_image, x, y)] = src_img.data[getImageOffset(src_img, x, y)];
//             prev_depth.data[getImageOffset(curr_depth, x, y)] = depth_map.data[getImageOffset(depth_map, x, y)];
//         }
//     }

//     // iter for each frame
//     for (int it = 0; it < frames; ++it)
//     {
//         // initialize curr variables
//         #pragma omp parallel for shared(curr_image, curr_depth)
//         for (int x = 0; x < X; ++x)
//         {
//             #pragma omp parallel for shared(curr_image, curr_depth)
//             for (int y = 0; y < Y; ++y)
//             {
//                 curr_image.data[getImageOffset(curr_image, x, y)].r = -1;
//                 curr_image.data[getImageOffset(curr_image, x, y)].g = -1;
//                 curr_image.data[getImageOffset(curr_image, x, y)].b = -1;

//                 curr_depth.data[getImageOffset(curr_depth, x, y)] = -1;
//             }
//         }
//         std::vector<std::tuple<std::tuple<int, int>, std::tuple<int, int>>> list_of_pairs;

//         for (auto pair : list_of_pairs)
//         {
//             auto start = std::get<0>(pair);
//             auto end = std::get<1>(pair);

//             int x_s = std::get<0>(start);
//             int y_s = std::get<1>(start);
//             int x_e = std::get<0>(end);
//             int y_e = std::get<1>(end);

//             float depth = prev_depth.data[getImageOffset(prev_depth, x_s, y_s)];
//             float vel = velocity(depth);
//             // stretch the line of pixels on len(list_of_pairs)*vel
//             // and find new pixels values via linear interpolation
//             // and put them into curr_image
//             int center = 0;
//             int s = 0;
//             int e = 0;
//             if (x_s != x_e)
//             {
//                 center = (x_s + x_e) / 2;
//                 s = x_s;
//                 e = x_e;
//             } else
//             {
//                 center = (y_s + y_e) / 2;
//                 s = y_s;
//                 e = y_e;
//             }
//             int size = list_of_pairs.size();
//             std::vector<std::tuple<float, std::tuple<float, float, float>>> new_coords;
//             for (int p = s; p <= e; ++p)
//             {
//                 auto pxl = prev_image.data[getImageOffset(prev_image, p, p)];
//                 new_coords.push_back(std::make_tuple((p-center)*vel + center, std::make_tuple(pxl.r, pxl.g, pxl.b)));
//             }
//             for (int x = x_s; x <= x_e; ++x)
//             {
//                 for (int y = y_s; y <= y_e; ++y_e)
//                 {
//                     auto pxl = curr_image.data[getImageOffset(prev_image, x, y)];
//                     // pxl = interpolate(new_coords, x, y);
//                     curr_image.data[getImageOffset(prev_image, x, y)] = pxl;
//                 }
//             }
//         }

//         #pragma omp parallel for shared(curr_image, curr_depth)
//         for (int x = 0; x < X; ++x)
//         {
//             #pragma omp parallel for shared(curr_image, curr_depth)
//             for (int y = 0; y < Y; ++y)
//             {                

//                 float depth = prev_depth.data[getImageOffset(prev_depth, x, y)];

//                 int nxt_x = compute_nxt(x, x_center, depth);
//                 int nxt_y = compute_nxt(y, y_center, depth);

//                 if (nxt_x < 0 || nxt_x >= X || nxt_y < 0 || nxt_y >= Y) // || prev_depth.data[getImageOffset(prev_depth, nxt_x, nxt_y)] < depth) // + epsilon)
//                 {
//                     continue;
//                 }

//                 curr_image.data[getImageOffset(curr_image, nxt_x, nxt_y)].r = prev_image.data[getImageOffset(prev_image, x, y)].r;
//                 curr_image.data[getImageOffset(curr_image, nxt_x, nxt_y)].g = prev_image.data[getImageOffset(prev_image, x, y)].g;
//                 curr_image.data[getImageOffset(curr_image, nxt_x, nxt_y)].b = prev_image.data[getImageOffset(prev_image, x, y)].b;
//                 curr_depth.data[getImageOffset(curr_depth, nxt_x, nxt_y)] = prev_depth.data[getImageOffset(prev_depth, x, y)];
//             }
//         }

//         bool res = true;
//         while (res)
//         {
//             res = false;
//             #pragma omp parallel for shared(curr_image, curr_depth)
//             for (int x = 0; x < X; ++x)
//             {
//                 #pragma omp parallel for shared(curr_image, curr_depth)
//                 for (int y = 0; y < Y; ++y)
//                 {
//                     if (curr_image.data[getImageOffset(curr_image, x, y)].r == -1)
//                     {
//                         auto hi = curr_image.data[getImageOffset(curr_image, x, std::min(y+1, Y-1))];
//                         auto lo = curr_image.data[getImageOffset(curr_image, x, std::max(y-1, 0))];
//                         auto le = curr_image.data[getImageOffset(curr_image, std::max(x-1, 0), y)];
//                         auto ri = curr_image.data[getImageOffset(curr_image, std::min(x+1, X-1), y)];

//                         curr_image.data[getImageOffset(curr_image, x, y)].r = interpolate(hi.r, lo.r, le.r, ri.r);
//                         curr_image.data[getImageOffset(curr_image, x, y)].g = interpolate(hi.g, lo.g, le.g, ri.g);
//                         curr_image.data[getImageOffset(curr_image, x, y)].b = interpolate(hi.b, lo.b, le.b, ri.b);
//                         if (curr_image.data[getImageOffset(curr_depth, x, y)].r == -1 || 
//                             curr_image.data[getImageOffset(curr_depth, x, y)].g == -1 ||
//                             curr_image.data[getImageOffset(curr_depth, x, y)].b == -1
//                             )
//                         {
//                             res = true;
//                         }
//                         float hi_d = curr_depth.data[getImageOffset(curr_depth, x, std::min(y+1, Y-1))];
//                         float lo_d = curr_depth.data[getImageOffset(curr_depth, x, std::max(y-1, 0))];
//                         float le_d = curr_depth.data[getImageOffset(curr_depth, std::max(x-1, 0), y)];
//                         float ri_d = curr_depth.data[getImageOffset(curr_depth, std::min(x+1, X), y)];

//                         curr_depth.data[getImageOffset(curr_depth, x, y)] = interpolate(hi_d, lo_d, le_d, ri_d);
//                         if (curr_depth.data[getImageOffset(curr_depth, x, y)] == -1)
//                         {
//                             res = true;
//                         }
//                     }
//                 }
//             }
//         }
        
//         // std::cout<< "pushing..." << std::endl;
//         auto img_to_be_pushed = ImageRGB(X, Y);
//         #pragma omp parallel for shared(img_to_be_pushed, curr_image)
//         for (int x = 0; x < X; ++x)
//         {
//             #pragma omp parallel for shared(img_to_be_pushed, curr_image)
//             for (int y = 0; y < Y; ++y)
//             {
//                 img_to_be_pushed.data[getImageOffset(img_to_be_pushed, x, y)].r = curr_image.data[getImageOffset(curr_image, x, y)].r;
//                 img_to_be_pushed.data[getImageOffset(img_to_be_pushed, x, y)].g = curr_image.data[getImageOffset(curr_image, x, y)].g;
//                 img_to_be_pushed.data[getImageOffset(img_to_be_pushed, x, y)].b = curr_image.data[getImageOffset(curr_image, x, y)].b;
//             }
//         }
//         output_frames.push_back(img_to_be_pushed);
//         std::swap(prev_image, curr_image);
//         std::swap(prev_depth, curr_depth);
//     }
    
// }

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
            // if (depth.data[getImageOffset(depth, x, y)] == INVALID_VALUE)
            // {
            //     px_disparity.data[getImageOffset(px_disparity, x, y)] = INVALID_VALUE;
            //     continue;
            // }
            
            float z = (depth.data[getImageOffset(depth, x, y)]) * (far_plane_mm - near_plane_mm)  - (far_plane_mm - near_plane_mm) / 2;
            px_disparity.data[getImageOffset(px_disparity, x, y)] = (iod_mm / px_size_mm) * (z / (screen_distance_mm + z));
        }
    }

    return px_disparity; // returns disparity measured in pixels
}