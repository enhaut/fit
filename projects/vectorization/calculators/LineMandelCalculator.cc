/**
 * @file LineMandelCalculator.cc
 * @author Samuel Dobron <xdobro23@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date 2023-10-26 11:00
 */
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <mm_malloc.h>
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>


#include "LineMandelCalculator.h"


LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
{
	data = (int *)calloc(height * width, sizeof(int));  // PERF: mm_calloc???
	for (int i = 0; i < height; i++)
	{
	  for (int j = 0; j < width; j++)
	  	*(data + i*width + j) = limit;
	}

	zImagf = (float *)_mm_malloc(width * sizeof(float), CACHE_LINE_SIZE);
	zRealf = (float *)_mm_malloc(width * sizeof(float), CACHE_LINE_SIZE);

	processed = (int *)_mm_malloc(width * sizeof(int), CACHE_LINE_SIZE);  // TODO: move prefill
}

LineMandelCalculator::~LineMandelCalculator() {
	free(data);
	_mm_free(zImagf);
	_mm_free(zRealf);
	_mm_free(processed);

	zImagf = NULL;
	zRealf = NULL;
	processed = NULL;
	data = NULL;
}

int * LineMandelCalculator::calculateMandelbrot () {
	float real, r2, i2, y, i_f = 0.0f, j_f = 0.0f;

	float x_start_f = x_start;
	float y_start_f = y_start;
	float dx_f = dx;
	float dy_f = dy;
	// ^^ retype doubles to floats to remove implicit recasting in loops

	float *Rlf = zRealf;
	float *Imf = zImagf;
	int *proc = processed;

	unsigned int i, j, k, processed_count;  // 20ms performance gain 
	for (i = 0; i < height/2; i++)
	{
		y = y_start_f + i_f * dy_f; // current imaginary value

		memset(processed, 0, width * sizeof(int));

		#pragma omp simd reduction(+: j) aligned(Rlf, Imf, proc: CACHE_LINE_SIZE)
		for (j = 0; j < width; j++)
		{
			Imf[j] = y;
			Rlf[j] = x_start_f + j * dx_f;
		}

		processed_count = 0;
	  	for (k = 0; k < limit; ++k)
		{
			#pragma omp simd reduction(+: processed_count, j_f, j) aligned(Rlf, Imf, proc: CACHE_LINE_SIZE)
		  	for (j = 0; j < width; j++)
	  	  	{
				// PERF: builtin cpu prefetcher is much faster than prefetching next cacheline manually

				real = x_start_f + j_f * dx_f;  // having it here is faster than moving it bellow if

		  	  	r2 = Rlf[j] * Rlf[j];
		  	  	i2 = Imf[j] * Imf[j];

		  	  	if (!proc[j] && r2 + i2 > 4.0f)
		  	  	{
					proc[j] = 1;
					processed_count++;
		  	  	  	*(data + i*width + j) = k;
		  	  	  	*(data + (height-1)*width-(i*width) + j) = k;
				}

		  	  	Imf[j] = 2.0f * Rlf[j] * Imf[j] + y;
		  	  	Rlf[j] = r2 - i2 + real;

		  	  	j_f++;
	  	  	}
	  	  	j_f = 0.0f;
			if (processed_count >= width)
			  	break;
		}
		i_f++;
	}
	
	return data;
}
