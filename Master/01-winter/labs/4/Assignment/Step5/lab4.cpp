/**
 * @file      lab4.cpp
 *
 * @author    Jiri Jaros \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            jarosjir@fit.vutbr.cz
 *
 * @brief     AVS - PC lab 4
 *            Parallel search in unsorted array.
 *
 * @version   2021
 *
 * @date      08 November  2020, 18:54 (created) \n
 * @date      05 November  2021, 18:54 (revised) \n
 *
 */

#include <unistd.h>
#include <cstdio>
#include <map>
#include <chrono>
#include <functional>
#include <omp.h>

using namespace std;

//--------------------------------------------------------------------------------------------------------------------//
//                                     Type definition Function prototypes                                            //
//--------------------------------------------------------------------------------------------------------------------//

/**
 * Parallel search
 */
template<size_t size>
size_t search(const size_t* array, size_t key);

/// Function pointer definition
using searchBenchmarkFnc = std::function<size_t(const size_t*, size_t)>;

//--------------------------------------------------------------------------------------------------------------------//
//                                     Global constants and benchmark maps                                            //
//--------------------------------------------------------------------------------------------------------------------//
// Number of tests
constexpr size_t nTests = 4;
// Size of arrays used
constexpr size_t benchmarkSizes[nTests] = {1000, 1000000, 10000000, 999999999};
// Keys to find
constexpr size_t keys[nTests] = {650, 758231, 4212157, 351233241};

// Number of benchmark repetitions
constexpr size_t testRept[nTests]       = {1000000, 10000, 1000, 5};
// Max array size
const size_t maxSizes = benchmarkSizes[nTests - 1];

// Map with benchmark sizes
std::map<size_t, searchBenchmarkFnc> searchBenchmarks =
{
  {benchmarkSizes[0], &search<benchmarkSizes[0]>},
  {benchmarkSizes[1], &search<benchmarkSizes[1]>},
  {benchmarkSizes[2], &search<benchmarkSizes[2]>},
  {benchmarkSizes[3], &search<benchmarkSizes[3]>},
};// end of matrixBenchmarks

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           Routines to be implemented                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Search in unsorted array
 * @param [in] array - Where to search
 * @param [in] key   - What to search.
 * @return Position
 */
template<size_t size>
size_t search(const size_t* array, size_t key)
{
  //------------------------------------------------------------------------------------------------------------------//
  // 1. Add correct pragmas                                                                                           //
  //------------------------------------------------------------------------------------------------------------------//
  size_t pos = size_t(-1);

  #pragma omp parallel for
  for (size_t i = 0; i < size; i++)
  {
    if (array[i] == key)
    {
      pos = i;
      #pragma omp cancel for
    }
  }
  return pos;
}// end of search
//----------------------------------------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 *  main function
 */
int main(int argc, char** argv)
{
  char hostName[31];
  gethostname(hostName, 30);
  printf("---------------------------------------------------\n");
  printf(" Parallel Search in unsorted array                 \n");
  printf(" Running on: %s\n", hostName);
  printf(" Number of cores: %d\n", omp_get_num_procs());
  printf("---------------------------------------------------\n");

  // Generate data
  size_t* array = new size_t[benchmarkSizes[nTests - 1]];
  for (int i = 0; i < benchmarkSizes[nTests - 1]; i++)
  {
    array[i] = i;
  }

  // run test
  for (size_t testId = 0; testId < nTests; testId++)
  {
    printf("  - Testing pool:            %9d\n", benchmarkSizes[testId]);
    printf("  - Key:                     %9lu\n", keys[testId]);
    printf("  - Number of repetitions:   %9d\n", testRept[testId]);

    auto startTime = std::chrono::high_resolution_clock::now();
    // Run benchmark
    size_t pos = 0;
    for (size_t rept = 0; rept < testRept[testId]; rept++)
    {
      #pragma noinline recursive
      pos = searchBenchmarks[benchmarkSizes[testId]](array, keys[testId]);
    }

    // Elapsed time
    const auto   endTime = std::chrono::high_resolution_clock::now();
    const double time    = (endTime - startTime) / std::chrono::milliseconds(1);

    // Log
    printf("  - Position of the key:     %9lu\n", pos);
    printf("  - Time to find the key:    %9.3f ms\n", time / testRept[testId]);

    printf("---------------------------------------------------\n");
  }

  delete [] array;
  return EXIT_SUCCESS;
}// end of main
//----------------------------------------------------------------------------------------------------------------------
