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
#include <string>
#include <vector>
#include <algorithm>

#include <stdlib.h>


#include "LineMandelCalculator.h"


LineMandelCalculator::LineMandelCalculator (unsigned matrixBaseSize, unsigned limit) :
	BaseMandelCalculator(matrixBaseSize, limit, "LineMandelCalculator")
{
	data = (int *)(calloc(height * width, sizeof(int)));  // PERF: mm_calloc???
}

LineMandelCalculator::~LineMandelCalculator() {
	free(data);
	data = NULL;
}

template <typename T>
static inline int mandelbrot(T real, T imag, int limit)
{
	T zReal = real;
	T zImag = imag;

	for (int i = 0; i < limit; ++i)
	{
		T r2 = zReal * zReal;
		T i2 = zImag * zImag;

		if (r2 + i2 > 4.0f)
			return i;

		zImag = 2.0f * zReal * zImag + imag;
		zReal = r2 - i2 + real;
	}
	return limit;
}


int * LineMandelCalculator::calculateMandelbrot () {
	memset(data, height * width, limit);

	for (int i = 0; i < height; i++)
	{
		float y = y_start + i * dy; // current imaginary value

		int *processed = (int *)calloc(width, sizeof(int));
		float *zImagf = (float *)malloc(width * sizeof(float));
		float *zRealf = (float *)malloc(width * sizeof(float));
	
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
				float real = x_start + j * dx;

		  	  	float r2 = zRealf[j] * zRealf[j];
		  	  	float i2 = zImagf[j] * zImagf[j];

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
