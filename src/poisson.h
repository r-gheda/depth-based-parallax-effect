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
    // compute the pixel coefficient based on the difference between the pixels

    // check if the pixels are in the image
    if (! (x_1 >= 0 && x_1 < img.width && y_1 >= 0 && y_1 < img.height &&
        x_2 >= 0 && x_2 < img.width && y_2 >= 0 && y_2 < img.height)) {
        return 0;
    }

    float diff = 0;
    auto pxl_1 = img.data[getImageOffset(img, x_1, y_1)];
    auto pxl_2 = img.data[getImageOffset(img, x_2, y_2)];

    diff = abs(pxl_1 - pxl_2);
    
    return exp(-beta * diff); 
}

ImageFloat solvePoisson(const ImageFloat& input_image, std::map<int, float>  scribbles, std::vector<std::vector<bool>> lookup, const int num_iters = NUM_ITERS)
{
    auto start = std::chrono::steady_clock::now();
    // Empty solution.
    auto I = ImageFloat(input_image.width, input_image.height);
    
    // Another solution for the alteranting updates.
    auto I_next = ImageFloat(input_image.width, input_image.height);
    
    // Initialize the solution with the input image.
    #pragma omp parallel for shared(I, I_next)
    for (int i = 0;i < I.width * I.height;++i)
    {
        I.data[i] = input_image.data[i];
        I_next.data[i] = input_image.data[i];
        // insert scribbles in the image 
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
        
        #pragma omp parallel for shared(I, I_next)
        for (int x = 0;x < X; ++x)
        {
            #pragma omp parallel for shared(I, I_next)
            for (int y = 0;y < Y; ++y)
            {
                // if the pixel is a scribble, skip it
                if (lookup[x][y]) {
                    continue;
                }

                // get neighbors values
                float prv_x = 0;
                float nxt_x = 0;
                float prv_y = 0;
                float nxt_y = 0;

                int den = 0;

                if (x > 0)
                {
                    prv_x = I.data[getImageOffset(I, x - 1, y)];
                    den++;
                }
                if (x < (X - 1))
                {
                    nxt_x = I.data[getImageOffset(I, x + 1, y)];
                    den++;
                }
                if (y > 0)
                {
                    prv_y = I.data[getImageOffset(I, x, y - 1)];
                    den++;
                }
                if (y < (Y - 1))
                {
                    nxt_y = I.data[getImageOffset(I, x, y + 1)];
                    den++;
                }

                // update the pixel value
                I_next.data[getImageOffset(I_next, x, y)] = ( prv_x + nxt_x + prv_y + nxt_y) / den;
            }
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
    
    // Initialize the solution with the input image.
    #pragma omp parallel for shared(I, I_next)
    for (int i = 0;i < I.width * I.height;++i)
    {
        I.data[i] = input_image.data[i];
        I_next.data[i] = input_image.data[i];
        // insert scribbles in the image
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
        
        #pragma omp parallel for shared(I, I_next)
        for (int x = 0;x < X; ++x)
        {
            #pragma omp parallel for shared(I, I_next)
            for (int y = 0;y < Y; ++y)
            {
                // if the pixel is a scribble, skip it
                if (lookup[x][y]) {
                    continue;
                }

                // get neighbors values and weights
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
                }
                if (x < (X - 1))
                {
                    nxt_x = I.data[getImageOffset(I, x + 1, y)];
                    w_nxt_x = pixel_coeff(x, y, x + 1, y, greyscale, beta);
                }
                if (y > 0)
                {
                    prv_y = I.data[getImageOffset(I, x, y - 1)];
                    w_prv_y = pixel_coeff(x, y, x, y - 1, greyscale, beta);
                }
                if (y < (Y - 1))
                {
                    nxt_y = I.data[getImageOffset(I, x, y + 1)];
                    w_nxt_y = pixel_coeff(x, y, x, y + 1, greyscale, beta);
                }

                // update the pixel value
                I_next.data[getImageOffset(I_next, x, y)] = ((w_prv_x * prv_x + w_nxt_x * nxt_x + w_prv_y * prv_y + w_nxt_y * nxt_y)) / (w_prv_x + w_nxt_x + w_prv_y + w_nxt_y);
            }
        }

        // Swaps the current and next solution so that the next iteration
        // uses the new solution as input and the previous solution as output.
        std::swap(I, I_next);
    }

    // After the last "swap", I is the latest solution.
    return I;
}
