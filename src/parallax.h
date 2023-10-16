#pragma once
#include "helpers.h"

#include <cmath>
#include <map>
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

int compute_nxt(int pxl, int center, float depth){
    if (pxl == center)
    {
        return pxl;
    }

    int diff = velocity(depth);

    if (pxl > center)
    {
        return pxl + diff;
    }

    return pxl - diff;
}

float interpolate(float hi, float lo, float le, float ri)
{
    float div = 4.0;
    if (hi < 0)
    {
        hi = 0;
        div -= 1;
    }
    if (lo < 0)
    {
        lo = 0;
        div -= 1;
    }
    if (le < 0)
    {
        le = 0;
        div -= 1;
    }
    if (ri < 0)
    {
        ri = 0;
        div -= 1;
    }
    if (div == 0)
    {
        return -1;
    }
    return (hi + lo + le + ri) / div;
}

void generate_parallax_frames(std::vector<ImageRGB>& output_frames, const ImageRGB& src_img, const ImageFloat& depth_map, int frames = 30, float epsilon = 1.0 / 255)
{
    // std::cout<< "start" << std::endl;
    // std::vector<ImageRGB> output_frames;
    // output_frames.reserve(frames);
    output_frames.push_back(src_img);

    const int X = src_img.width;
    const int Y = src_img.height;

    auto curr_image = ImageRGB(X, Y);
    auto prev_image = ImageRGB(X, Y);
    auto curr_depth = ImageFloat(X, Y);
    auto prev_depth = ImageFloat(X, Y);

    // std::cout<< "first loop" << std::endl;
    #pragma omp parallel for shared(curr_image, curr_depth)
    for (int x = 0; x < X; ++x)
    {
        #pragma omp parallel for shared(curr_image, curr_depth)
        for (int y = 0; y < Y; ++y)
        {
            prev_image.data[getImageOffset(curr_image, x, y)] = src_img.data[getImageOffset(src_img, x, y)];
            prev_depth.data[getImageOffset(curr_depth, x, y)] = depth_map.data[getImageOffset(depth_map, x, y)];
            // depth_values[(int) (depth_map.data[getImageOffset(depth_map, x, y)]*255)].push_back(std::make_tuple(x, y));
        }
    }

    // std::cout<< "second loop" << std::endl;
    for (int it = 0; it < frames; ++it)
    {
        // std::cout<< "iter: " << it << std::endl;
        // std::cout<< "setting up currs..." << std::endl;
        #pragma omp parallel for shared(curr_image, curr_depth)
        for (int x = 0; x < X; ++x)
        {
            #pragma omp parallel for shared(curr_image, curr_depth)
            for (int y = 0; y < Y; ++y)
            {
                curr_image.data[getImageOffset(curr_image, x, y)].r = -1;
                curr_image.data[getImageOffset(curr_image, x, y)].g = -1;
                curr_image.data[getImageOffset(curr_image, x, y)].b = -1;

                curr_depth.data[getImageOffset(curr_depth, x, y)] = -1;
            }
        }

        // std::cout<< "zooming..." << std::endl;
        #pragma omp parallel for shared(curr_image, curr_depth)
        for (int x = 0; x < X; ++x)
        {
            #pragma omp parallel for shared(curr_image, curr_depth)
            for (int y = 0; y < Y; ++y)
            {

                int x_center = X / 2;
                int y_center = Y / 2;

                float depth = prev_depth.data[getImageOffset(prev_depth, x, y)];

                int nxt_x = compute_nxt(x, x_center, depth);
                int nxt_y = compute_nxt(y, y_center, depth);

                if (nxt_x < 0 || nxt_x >= X || nxt_y < 0 || nxt_y >= Y) // || prev_depth.data[getImageOffset(prev_depth, nxt_x, nxt_y)] < depth) // + epsilon)
                {
                    continue;
                }

                curr_image.data[getImageOffset(curr_image, nxt_x, nxt_y)] = prev_image.data[getImageOffset(prev_image, x, y)];
                curr_depth.data[getImageOffset(curr_depth, nxt_x, nxt_y)] = prev_depth.data[getImageOffset(prev_depth, x, y)];
            }
        }

        // std::cout<< "interpolating..." << std::endl;
        bool res = true;
        while (res)
        {
            res = false;
            #pragma omp parallel for shared(curr_image, curr_depth)
            for (int x = 0; x < X; ++x)
            {
                #pragma omp parallel for shared(curr_image, curr_depth)
                for (int y = 0; y < Y; ++y)
                {
                    if (curr_image.data[getImageOffset(curr_image, x, y)].r == -1)
                    {
                        //// std::cout<< "rgb..." << std::endl;
                        auto hi = curr_image.data[getImageOffset(curr_image, x, std::min(y+1, Y-1))];
                        auto lo = curr_image.data[getImageOffset(curr_image, x, std::max(y-1, 0))];
                        auto le = curr_image.data[getImageOffset(curr_image, std::max(x-1, 0), y)];
                        auto ri = curr_image.data[getImageOffset(curr_image, std::min(x+1, X-1), y)];

                        //// std::cout<< "neighbors loaded" << std::endl;
                        curr_image.data[getImageOffset(curr_image, x, y)].r = interpolate(hi.r, lo.r, le.r, ri.r);
                        curr_image.data[getImageOffset(curr_image, x, y)].g = interpolate(hi.g, lo.g, le.g, ri.g);
                        curr_image.data[getImageOffset(curr_image, x, y)].b = interpolate(hi.b, lo.b, le.b, ri.b);
                        if (curr_image.data[getImageOffset(curr_depth, x, y)].r == -1 || 
                            curr_image.data[getImageOffset(curr_depth, x, y)].g == -1 ||
                            curr_image.data[getImageOffset(curr_depth, x, y)].b == -1
                            )
                        {
                            res = true;
                        }
                        //// std::cout<< "depth..." << std::endl;
                        float hi_d = curr_depth.data[getImageOffset(curr_depth, x, std::min(y+1, Y-1))];
                        float lo_d = curr_depth.data[getImageOffset(curr_depth, x, std::max(y-1, 0))];
                        float le_d = curr_depth.data[getImageOffset(curr_depth, std::max(x-1, 0), y)];
                        float ri_d = curr_depth.data[getImageOffset(curr_depth, std::min(x+1, X), y)];

                        curr_depth.data[getImageOffset(curr_depth, x, y)] = interpolate(hi_d, lo_d, le_d, ri_d);
                        if (curr_depth.data[getImageOffset(curr_depth, x, y)] == -1)
                        {
                            //// std::cout<< 1 << std::endl;
                            res = true;
                        }
                        //// std::cout<< curr_depth.data[getImageOffset(curr_image, x,y)] << std::endl;
                    }
                }
            }
        }
        
        // std::cout<< "pushing..." << std::endl;
        auto img_to_be_pushed = ImageRGB(X, Y);
        #pragma omp parallel for shared(img_to_be_pushed, curr_image)
        for (int x = 0; x < X; ++x)
        {
            #pragma omp parallel for shared(img_to_be_pushed, curr_image)
            for (int y = 0; y < Y; ++y)
            {
                img_to_be_pushed.data[getImageOffset(img_to_be_pushed, x, y)].r = curr_image.data[getImageOffset(curr_image, x, y)].r;
                img_to_be_pushed.data[getImageOffset(img_to_be_pushed, x, y)].g = curr_image.data[getImageOffset(curr_image, x, y)].g;
                img_to_be_pushed.data[getImageOffset(img_to_be_pushed, x, y)].b = curr_image.data[getImageOffset(curr_image, x, y)].b;
            }
        }
        // std::cout<< "actually pushing..." << std::endl;
        output_frames.push_back(img_to_be_pushed);
        // img_to_be_pushed.writeToFile("../frames/output_" + std::to_string(it) + ".png");
        std::swap(prev_image, curr_image);
        std::swap(prev_depth, curr_depth);
        // std::cout<< "swapped" << std::endl;
    }
    
    // std::cout<< "returning..." << std::endl;
}
