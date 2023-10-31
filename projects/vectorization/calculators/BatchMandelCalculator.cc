/**
 * @file BatchMandelCalculator.cc
 * @author Samuel Dobron <xdobro23@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over small batches
 * @date 2023-10-25 20:23
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

#define BATCH_SIZE 64

BatchMandelCalculator::BatchMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator")
{
	data = (int *)malloc((height * width + BATCH_SIZE + 1) * sizeof(int));  // unaligned is faster than _mm_malloc
	
	#pragma omp simd
	for (int i = 0; i < height * width; i++)
	  	*(data + i) = limit;  // faster than memset (not sure why)

	zImagf = (float *)_mm_malloc(width * sizeof(float), CACHE_LINE_SIZE);
	zRealf = (float *)_mm_malloc(width * sizeof(float), CACHE_LINE_SIZE);
	batchReal = (float *)_mm_malloc(width * sizeof(float), CACHE_LINE_SIZE);
	// NOTE: Rows/cols are aligned to power of 2, therefore
	// multiple rows/cols cannot overlap across batches => x/y is going to be the 
	// same for whole batch (across iteRlfations).
	
	processed = (int *)_mm_malloc(width * sizeof(int), CACHE_LINE_SIZE);  // TODO: move prefill
}

BatchMandelCalculator::~BatchMandelCalculator() {
	_mm_free(data);
	_mm_free(zImagf);
	_mm_free(zRealf);
	_mm_free(batchReal);
	_mm_free(processed);

	data = NULL;
	zImagf = NULL;
	zRealf = NULL;
	batchReal = NULL;
	processed = NULL;
}

int * BatchMandelCalculator::calculateMandelbrot ()
{
	float x, y, r2, i2;
	int early_end;

	// ^^ retype doubles to floats to remove implicit recasting in loops
	float *Rlf = zRealf;
	float *Imf = zImagf;
	float *bReal = batchReal;

	for (int batch = 0; batch < height * width / 2; batch += BATCH_SIZE)
	{
		memset(processed, 0, BATCH_SIZE * sizeof(int));
		early_end = 0;

		y = y_start + batch / width * dy;
		x = x_start + (batch % width) * dx;
		// ^^ for explanation of having same x/y for whole batch see initializer
	
		#pragma omp simd reduction(+:x)
		for (int j = 0; j < BATCH_SIZE; j++)  // initialize starting values
		{
			bReal[j] = x;
			Rlf[j] = x;
			Imf[j] = y;
		}
		for (int k = 0; k < limit; ++k)
		{
			__builtin_prefetch(Rlf + BATCH_SIZE);
			__builtin_prefetch(Imf + BATCH_SIZE);
			__builtin_prefetch(bReal + BATCH_SIZE);
			__builtin_prefetch(processed + BATCH_SIZE);

			#pragma omp simd reduction(+:early_end) aligned(Rlf, Imf, bReal: CACHE_LINE_SIZE)
			for (int j = 0; j < BATCH_SIZE; j++)
			{

				r2 = Rlf[j] * Rlf[j];
				i2 = Imf[j] * Imf[j];
				
				if (!processed[j] && r2 + i2 > 4.0f)
				{
					processed[j] = 1;
					early_end += 1;
		  	  	  	*(data + batch + j) = k;
				}else{
		  	  	  	Imf[j] *= 2.0f * Rlf[j];
		  	  	  	Imf[j] += y;
		  	  	  	Rlf[j] = bReal[j] + r2 - i2;
		  	  	}
			}
			if (early_end >= BATCH_SIZE)
			  break;  // got values for whole batch
		}
	}

	int *end_base = data + height * width - width;
	for (int row = 0; row < height / 2; row++)
	  std::memcpy(end_base - row*width, data + row * width, width * sizeof(int));

	return data;
}
