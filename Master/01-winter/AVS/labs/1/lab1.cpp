/**
 * @file      lab1.cpp
 *
 * @author    Jiri Jaros \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            jarosjir@fit.vutbr.cz
 *
 * @brief     AVS - PC lab 1
 *            Examples for performance measurement in Intel tools
 *            Vector operations
 *
 *
 * @version   2021
 *
 * @date      11 October   2020, 11:51 (created) \n
 * @date      11 October   2020, 11:51 (created) \n
 *
 */

#include <unistd.h>
#include <immintrin.h>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <functional>

using namespace std;

//--------------------------------------------------------------------------------------------------------------------//
//                                     Function prototypes                                                            //
//--------------------------------------------------------------------------------------------------------------------//
template<size_t size>
void vectorAdd(float* c, float* a, float* b);

template<size_t size>
void vectorDot(float* c, float* a, float* b);

//--------------------------------------------------------------------------------------------------------------------//
//                                     Global constants and benchmark maps                                            //
//--------------------------------------------------------------------------------------------------------------------//
// Number of tests
constexpr size_t nTests = 5;
// Size of arrays used
constexpr size_t arraySizes[nTests] = {64,                 //  256B / array =  768B (L1 fit)
                                       1   * 1024,         // 4096B / array =  12KB (L1 fit)
                                       24   * 1024,        //  96KB / array = 288KB (L2 fit)
                                       512 * 1024,         //   2MB / array =   6MB (L3 fit)
                                       4   * 1024 *1024};  //  16MB / array =  48MB (RAM fit)

// Number of benchmark repetitions
constexpr size_t testRept[nTests]   = {10000000, 1000000, 100000, 1000, 100};
// Max array size
constexpr size_t maxSize = arraySizes[nTests - 1];


/// Pointer to computeMainLoop method.
using VectorBenchmarkFnc = std::function<void(float*, float*, float*)>;

std::map<size_t, VectorBenchmarkFnc> addBenchmarks =
{
  {arraySizes[0], &vectorAdd<arraySizes[0]>},
  {arraySizes[1], &vectorAdd<arraySizes[1]>},
  {arraySizes[2], &vectorAdd<arraySizes[2]>},
  {arraySizes[3], &vectorAdd<arraySizes[3]>},
  {arraySizes[4], &vectorAdd<arraySizes[4]>},
};


std::map<size_t, VectorBenchmarkFnc> dotBenchmarks =
{
  {arraySizes[0], &vectorDot<arraySizes[0]>},
  {arraySizes[1], &vectorDot<arraySizes[1]>},
  {arraySizes[2], &vectorDot<arraySizes[2]>},
  {arraySizes[3], &vectorDot<arraySizes[3]>},
  {arraySizes[4], &vectorDot<arraySizes[4]>},
};

/**
 * Generate random data to fill the arrays
 * @param [out] array - Array to be filled in
 * @param [in] size   - Size of the array
 */
void generateData(float* array,
                  size_t size)
{
  // Generate random data as input
  for (size_t i = 0; i < size; i++)
  {
    array[i] = float(rand()) / float(RAND_MAX);
  }
}// end of generateData
//----------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------//
//                                                      Benchmarks                                                    //
//--------------------------------------------------------------------------------------------------------------------//


/**
 * Vector addition
 * @param c    - Output array
 * @param a    - Input array A
 * @param b    - Input array B
 * @param size - Size of the arrays

 */
template<size_t size>
void vectorAdd(float* c,
               float* a,
               float* b)
{
  for (size_t i = 0 ; i < size; i++)
  {
    c[i] = a[i] + b[i];
  }
}// end of vectorAdd
//----------------------------------------------------------------------------------------------------------------------

/**
 * Vector dot product
 * @param c    - Single value in the element [0]
 * @param a    - Input array A
 * @param b    - Input array B
 * @param size - Size of the arrays

 */
template<size_t size>
void vectorDot(float* c,
               float* a,
               float* b)
{
  float res = 0;
  for (size_t i = 0 ; i < size; i++)
  {
    res += a[i] * b[i];
  }
  c[0] = res;
}// end of vectorDot
//----------------------------------------------------------------------------------------------------------------------

/**
 *  main function
 */
int main(int argc, char** argv)
{

  /// Necessary arrays
  float* arrayA = (float *) _mm_malloc(maxSize * sizeof(float), 64);
  float* arrayB = (float *) _mm_malloc(maxSize * sizeof(float), 64);
  float* arrayC = (float *) _mm_malloc(maxSize * sizeof(float), 64);

  if (!(arrayA &&  arrayB && arrayC))
  {
    printf(" !!! Not enough memory to run the test !!! \n");
    exit(EXIT_FAILURE);
  }


  char hostName[31];
  gethostname(hostName, 30);
  printf("--------------------------------------------\n");
  printf("Program to test HW performance counters\n");
  printf("Running on: %s\n", hostName);

  printf("--------------------------------------------\n");
  printf("Generating random data... "); fflush(stdout);
  generateData(arrayA, maxSize);
  generateData(arrayB, maxSize);
  printf("Done\n");

  //------------------------------------------------------------------------------------------------------------------//
  // Test 1: Vector addition                                                                                          //
  //------------------------------------------------------------------------------------------------------------------//

  printf("Test 1: Vector addition\n");
  for (int testId = 0; testId < nTests; testId++)
  {
    printf("  - size = %8ld, rept = %8ld ... ", arraySizes[testId], testRept[testId]); fflush(stdout);
    for (size_t rept = 0; rept < testRept[testId]; rept++)
    {
      #pragma noinline recursive
      addBenchmarks[arraySizes[testId]](arrayC, arrayA, arrayB);
    }
    printf("Done\n");
  }


  //------------------------------------------------------------------------------------------------------------------//
  // Test 2: Vector dot product                                                                                       //
  //------------------------------------------------------------------------------------------------------------------//
  printf("Test 2: Vector dot product\n");
  for (int testId = 0; testId < nTests; testId++)
  {
    printf("  - size = %8ld, rept = %8ld ... ", arraySizes[testId], testRept[testId]); fflush(stdout);
    for (size_t rept = 0; rept < testRept[testId]; rept++)
    {
      #pragma noinline recursive
      dotBenchmarks[arraySizes[testId]](arrayC, arrayA, arrayB);
    }
    printf("Done\n");
  }


  // Free data
  _mm_free(arrayA);
  _mm_free(arrayB);
  _mm_free(arrayC);

  return EXIT_SUCCESS;
}// end of main
//----------------------------------------------------------------------------------------------------------------------
