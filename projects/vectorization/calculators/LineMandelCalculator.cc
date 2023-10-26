/**
 * @file LineMandelCalculator.cc
 * @author FULL NAME <xlogin00@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over lines
 * @date DATE
 */
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

	zImagf = (float *)_mm_malloc(width * sizeof(float), 4);
	zRealf = (float *)_mm_malloc(width * sizeof(float), 4);

	processed = (int *)_mm_malloc(width * sizeof(int), 4);
}

LineMandelCalculator::~LineMandelCalculator() {
	free(data);
	data = NULL;
}

int * LineMandelCalculator::calculateMandelbrot () {
	float real, r2, i2, y;

	for (int i = 0; i < height; i++)
	{
		y = y_start + i * dy; // current imaginary value

		memset(processed, 0, width * sizeof(int));
		memset(zImagf, y, width * sizeof(float));
		for (int x = 0; x < width; x++)
		{
			zImagf[x] = y;
			zRealf[x] = x_start + x * dx;
		}
	  	for (int k = 0; k < limit; ++k)
		{
			#pragma omp simd
		  	for (int j = 0; j < width; j++)
	  	  	{
				real = x_start + j * dx;

		  	  	r2 = zRealf[j] * zRealf[j];
		  	  	i2 = zImagf[j] * zImagf[j];

		  	  	if (r2 + i2 > 4.0f && !processed[j])
		  	  	{
					processed[j] = 1;
		  	  	  	*(data + i*width + j) = k;
				}

		  	  	zImagf[j] = 2.0f * zRealf[j] * zImagf[j] + y;
		  	  	zRealf[j] = r2 - i2 + real;
	  	  	}
	
		}
	}
	
	return data;
}
