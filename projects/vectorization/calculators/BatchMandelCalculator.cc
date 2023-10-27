/**
 * @file BatchMandelCalculator.cc
 * @author FULL NAME <xlogin00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date DATE
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <mm_malloc.h>

#include <stdlib.h>
#include <stdexcept>

#include "BatchMandelCalculator.h"

#define BATCH_SIZE 96
BatchMandelCalculator::BatchMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator")
{
	data = (int *)malloc((height * width + BATCH_SIZE + 1) * sizeof(int));  // unaligned is faster than _mm_malloc
	
	#pragma omp simd
	for (int i = 0; i < height * width; i++)
	  	*(data + i) = limit;  // faster than memset (not sure why)

	zImagf = (float *)_mm_malloc(width * sizeof(float), 64);
	zRealf = (float *)_mm_malloc(width * sizeof(float), 64);
	
	processed = (int *)_mm_malloc(width * sizeof(int), 64);  // TODO: move prefill

}

BatchMandelCalculator::~BatchMandelCalculator() {
	_mm_free(data);
	_mm_free(zImagf);
	_mm_free(zRealf);
	_mm_free(batchReal);
	_mm_free(processed);
}

int * BatchMandelCalculator::calculateMandelbrot (){
    for (int batch = 0; batch < height * width + BATCH_SIZE; batch += BATCH_SIZE)
    {
        for (int j = 0; j< BATCH_SIZE; j++)
          processed[j] = 0;

        for (int j = 0; j < BATCH_SIZE; j++)
        {
            float y = y_start + int((batch+j) / width) * dy;
            float x = x_start + int((batch+j) % width) * dx;
            
            zRealf[j] = x;
            zImagf[j] = y;
        }
        for (int k = 0; k < limit; ++k)
        {

            for (int j = 0; j < BATCH_SIZE; j++)
            {
                float r2 = zRealf[j] * zRealf[j];
                float i2 = zImagf[j] * zImagf[j];

                // float real = x_start + 
                float real = x_start + int((batch+j) % width) * dx;
                float y = y_start + int((batch+j) / width) * dy;
                
                if (!processed[j] && r2 + i2 > 4.0f)
                {
                    processed[j] = 1;

                          *(data + batch + j) = k;
                          // *(data + (height-1)*width-(i*width) + j) = k;
                }
                    zImagf[j] = 2.0f * zRealf[j] * zImagf[j] + y;
                    zRealf[j] = r2 - i2 + real;
            }
        }
        
    }
    return data;
}
