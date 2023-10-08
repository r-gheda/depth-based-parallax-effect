#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <span>
#include <tuple>
#include <map>
#include <vector>

#include "helpers.h"
#include <iostream>
#include <math.h>
#include <cstdlib>
#include "omp.h"
#include <chrono>

const float BETA = 20.0;
const int NUM_ITERS = 4000;
const float TOLERANCE = 1 / 255.0f;
const float RGB_TO_GREY[3] = {0.2989, 0.5870, 0.1140};

template<typename T>
int getImageOffset(const Image<T>& image, int x, int y)
{
    return x + (y * image.width);
}

float pixel_coeff(int x_1, int y_1, int x_2, int y_2, const ImageFloat& img, float beta){
    if (! (x_1 >= 0 && x_1 < img.width && y_1 >= 0 && y_1 < img.height &&
        x_2 >= 0 && x_2 < img.width && y_2 >= 0 && y_2 < img.height)) {
        return 0;
    }

    float diff = 0;
    auto pxl_1 = img.data[getImageOffset(img, x_1, y_1)];
    auto pxl_2 = img.data[getImageOffset(img, x_2, y_2)];

    diff = abs(pxl_1 - pxl_2);
    
    // diff = abs((pxl_1.r + pxl_1.g + pxl_1.b) - (pxl_2.r + pxl_2.g + pxl_2.b));
    return exp(-beta * diff); 
}

ImageFloat solvePoisson(const ImageFloat& input_image, std::map<int, float>  scribbles, std::vector<std::vector<bool>> lookup, const int num_iters = NUM_ITERS)
{
    auto start = std::chrono::steady_clock::now();
    // Empty solution.
    auto I = ImageFloat(input_image.width, input_image.height);
    
    // Another solution for the alteranting updates.
    auto I_next = ImageFloat(input_image.width, input_image.height);
    
    #pragma omp parallel for shared(I, I_next)
    for (int i = 0;i < I.width * I.height;++i)
    {
        I.data[i] = input_image.data[i];
        I_next.data[i] = input_image.data[i];
        
        if (lookup[i % I.width][i / I.width]) {
            I.data[i] = scribbles[i] ; // / 255.0f;
            I_next.data[i] = scribbles[i] ; // / 255.0f;
        }
    }

    // Iterative solver.
    for (auto iter = 0; iter < num_iters; iter++)
    {

        int X = input_image.width;
        int Y = input_image.height;
        
        std::vector<std::vector<int>> new_lookups;
        #pragma omp parallel for shared(I, I_next)
        for (int x = 0;x < X; ++x)
        {
            #pragma omp parallel for shared(I, I_next)
            for (int y = 0;y < Y; ++y)
            {
                if (lookup[x][y]) {
                    // I_next.data[getImageOffset(I_next, x, y)] = scribbles[getImageOffset(input_image, x, y)] ; // / 255.0f;
                    continue;
                }

                float prv_x = 0;
                float nxt_x = 0;
                float prv_y = 0;
                float nxt_y = 0;

                if (x > 0)
                {
                    prv_x = I.data[getImageOffset(I, x - 1, y)];
                    if ((lookup[x-1][y]) and ((abs(prv_x - I.data[getImageOffset(I, x, y)])) < TOLERANCE)) {
                        I.data[getImageOffset(I, x, y)] = prv_x;
                        I_next.data[getImageOffset(I_next, x, y)] = prv_x;
                        std::vector<int> tmp = {x, y};
                        new_lookups.push_back(tmp);
                        continue;
                    }
                }
                if (x < (X - 1))
                {
                    nxt_x = I.data[getImageOffset(I, x + 1, y)];
                    if ((lookup[x+1][y]) and ((abs(nxt_x - I.data[getImageOffset(I, x, y)])) < TOLERANCE)) {
                        I.data[getImageOffset(I, x, y)] = nxt_x;
                        I_next.data[getImageOffset(I_next, x, y)] = nxt_x;
                        std::vector<int> tmp = {x, y};
                        new_lookups.push_back(tmp);
                        continue;
                    }
                }
                if (y > 0)
                {
                    prv_y = I.data[getImageOffset(I, x, y - 1)];
                    if ((lookup[x][y-1]) and ((abs(prv_y - I.data[getImageOffset(I, x, y)])) < TOLERANCE)) {
                        I.data[getImageOffset(I, x, y)] = prv_y;
                        I_next.data[getImageOffset(I_next, x, y)] = prv_y;
                        std::vector<int> tmp = {x, y};
                        new_lookups.push_back(tmp);
                        continue;
                    }
                }
                if (y < (Y - 1))
                {
                    nxt_y = I.data[getImageOffset(I, x, y + 1)];
                    if ((lookup[x][y+1]) and ((abs(nxt_y - I.data[getImageOffset(I, x, y)])) < TOLERANCE)) {
                        I.data[getImageOffset(I, x, y)] =   nxt_y;
                        I_next.data[getImageOffset(I_next, x, y)] = nxt_y;
                        std::vector<int> tmp = {x, y};
                        new_lookups.push_back(tmp);
                        continue;
                    }
                }


                I_next.data[getImageOffset(I_next, x, y)] = ( prv_x + nxt_x + prv_y + nxt_y) / 4.0;
            }
        }

        for (auto i = 0; i < new_lookups.size(); i++) {
            lookup[new_lookups[i][0]][new_lookups[i][1]] = true;
        }

        // Swaps the current and next solution so that the next iteration
        // uses the new solution as input and the previous solution as output.
        std::swap(I, I_next);
    }

    return I;
}


ImageFloat solveAnisotropic(const ImageFloat& input_image, const ImageFloat& greyscale, std::map<int, float>  scribbles, std::vector<std::vector<bool>> lookup, const int num_iters = NUM_ITERS, const float beta = BETA)
{
    // Empty solution.
    auto I = ImageFloat(input_image.width, input_image.height);
    
    // Another solution for the alteranting updates.
    auto I_next = ImageFloat(input_image.width, input_image.height);
    
    #pragma omp parallel for shared(I, I_next)
    for (int i = 0;i < I.width * I.height;++i)
    {
        I.data[i] = input_image.data[i];
        I_next.data[i] = input_image.data[i];
        
        if (lookup[i % I.width][i / I.width]) {
            I.data[i] = scribbles[i] ;
            I_next.data[i] = scribbles[i] ;
        }
    }

    // Iterative solver.
    for (auto iter = 0; iter < num_iters; iter++)
    {
        int X = input_image.width;
        int Y = input_image.height;

        std::vector<std::vector<int>> new_lookups;
        
        #pragma omp parallel for shared(I, I_next)
        for (int x = 0;x < X; ++x)
        {
            #pragma omp parallel for shared(I, I_next)
            for (int y = 0;y < Y; ++y)
            {
                if (lookup[x][y]) {
                    continue;
                }
                float prv_x = 0;
                float nxt_x = 0;
                float prv_y = 0;
                float nxt_y = 0;

                float w_prv_x = 0;
                float w_nxt_x = 0;
                float w_prv_y = 0;
                float w_nxt_y = 0;

                if (x > 0)
                {
                    prv_x = I.data[getImageOffset(I, x - 1, y)];
                    w_prv_x = pixel_coeff(x, y, x - 1, y, greyscale, beta);
                    // if ((lookup[x-1][y]) and (abs(prv_x - I.data[getImageOffset(I, x, y)]) < TOLERANCE)) {
                    //     I.data[getImageOffset(I, x, y)] = prv_x;
                    //     I_next.data[getImageOffset(I, x, y)] = prv_x;
                    //     std::vector<int> tmp = {x, y};
                    //     new_lookups.push_back(tmp);
                    //     continue;
                    // }
                }
                if (x < (X - 1))
                {
                    nxt_x = I.data[getImageOffset(I, x + 1, y)];
                    w_nxt_x = pixel_coeff(x, y, x + 1, y, greyscale, beta);
                    // if ((lookup[x+1][y]) and (abs(nxt_x - I.data[getImageOffset(I, x, y)]) < TOLERANCE)) {
                    //     I.data[getImageOffset(I, x, y)] = nxt_x;
                    //     I_next.data[getImageOffset(I, x, y)] = nxt_x;
                    //     std::vector<int> tmp = {x, y};
                    //     new_lookups.push_back(tmp);
                    //     continue;
                    // }
                }
                if (y > 0)
                {
                    prv_y = I.data[getImageOffset(I, x, y - 1)];
                    w_prv_y = pixel_coeff(x, y, x, y - 1, greyscale, beta);
                    // if ((lookup[x][y-1]) and (abs(prv_y - I.data[getImageOffset(I, x, y)]) < TOLERANCE)) {
                    //     I.data[getImageOffset(I, x, y)] = prv_y;
                    //     I_next.data[getImageOffset(I, x, y)] = prv_y;
                    //     std::vector<int> tmp = {x, y};
                    //     new_lookups.push_back(tmp);
                    //     continue;
                    // }
                }
                if (y < (Y - 1))
                {
                    nxt_y = I.data[getImageOffset(I, x, y + 1)];
                    w_nxt_y = pixel_coeff(x, y, x, y + 1, greyscale, beta);
                    // if ((lookup[x][y+1]) and ((abs(nxt_y - I.data[getImageOffset(I, x, y)])) < TOLERANCE)) {
                    //     I.data[getImageOffset(I, x, y)] =   nxt_y;
                    //     I_next.data[getImageOffset(I_next, x, y)] = nxt_y;
                    //     std::vector<int> tmp = {x, y};
                    //     new_lookups.push_back(tmp);
                    //     continue;
                    // }
                }

                I_next.data[getImageOffset(I_next, x, y)] = ((w_prv_x * prv_x + w_nxt_x * nxt_x + w_prv_y * prv_y + w_nxt_y * nxt_y)) / (w_prv_x + w_nxt_x + w_prv_y + w_nxt_y);
            }
        }
        
        #pragma omp parallel for shared(lookup, new_lookups)
        for (auto i = 0; i < new_lookups.size(); i++) {
            lookup[new_lookups[i][0]][new_lookups[i][1]] = true;
        }

        // Swaps the current and next solution so that the next iteration
        // uses the new solution as input and the previous solution as output.
        std::swap(I, I_next);
    }

    // After the last "swap", I is the latest solution.
    return I;
}
