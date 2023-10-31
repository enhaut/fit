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
	float real, r2, i2, y, i_f, j_f;

	float x_start_f = x_start;
	float dx_f = dx;
	float dy_f = dy;
	// ^^ retype doubles to floats to remove implicit recasting in loops

	float *Rlf = zRealf;
	float *Imf = zImagf;
	int *proc = processed;

	i_f = 0.0f;
	j_f = 0.0f;
	
	for (int i = 0; i < height/2; i++)
	{
		y = y_start + i_f * dy; // current imaginary value

		memset(processed, 0, width * sizeof(int));

		#pragma omp simd aligned(Rlf, Imf, proc: CACHE_LINE_SIZE)
		for (int x = 0; x < width; x++)
		{
			Imf[x] = y;
			Rlf[x] = x_start_f + x * dx_f;
		}

		uint processed_count = 0;
	  	for (int k = 0; k < limit; ++k)
		{
			#pragma omp simd reduction(+: processed_count, j_f) aligned(Rlf, Imf, proc: CACHE_LINE_SIZE)
		  	for (int j = 0; j < width; j++)
	  	  	{
				__builtin_prefetch(Rlf + 1);
				__builtin_prefetch(Imf + 1);
				__builtin_prefetch(proc + 1);

				real = x_start_f + j_f * dx_f;

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
