/**
 * @file      lab5.cpp
 *
 * @author    Jiri Jaros \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            jarosjir@fit.vutbr.cz
 *
 * @brief     AVS - PC lab 5
 *            Parallel search in sorted array.
 *
 * @version   2021
 *
 * @date      21 November  2020, 19:01 (created) \n
 * @date      18 November  2021, 22:07 (revised) \n
 *
 */

#include <unistd.h>
#include <cstdio>
#include <map>
#include <chrono>
#include <functional>
#include <omp.h>
#include <algorithm>

using namespace std;

//--------------------------------------------------------------------------------------------------------------------//
//                                     Type definition Function prototypes                                            //
//--------------------------------------------------------------------------------------------------------------------//
constexpr size_t NOT_FOUND = size_t(-1);

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
constexpr size_t testRept[nTests]       = {100000, 100000, 100000, 10000};
// Max array size
const size_t maxSizes = benchmarkSizes[nTests - 1];

// Map with benchmark sizes
std::map<size_t, searchBenchmarkFnc> searchBenchmarks =
{
  {benchmarkSizes[0], &search<benchmarkSizes[0]>},
  {benchmarkSizes[1], &search<benchmarkSizes[1]>},
  {benchmarkSizes[2], &search<benchmarkSizes[2]>},
  {benchmarkSizes[3], &search<benchmarkSizes[3]>},
};// end of searchBenchmarks

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           Routines to be implemented                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Binary search
 * @param [in] array - Array
 * @param [in] left  - Left sentinel
 * @param [in] right - Right sentinel
 * @param [in] key   - Key to find
 * @return Position of the key
 */
size_t binarySearch(const size_t* array,
                    const size_t  left,
                    const size_t  right,
                    const size_t  key)
{
  //------------------------------------------------------------------------------------------------------------------//
  // 2. Add correct pragmas                                                                                           //
  //    Use pragma omp cancel if the key was found                                                                    //
  //------------------------------------------------------------------------------------------------------------------//

  // sequential threshold
  constexpr size_t seqThreshold = 1024;

  // position not found
  size_t pos = NOT_FOUND;

  // Recursion left
  if (left <= right)
  {
    int middle = (left + right)/2;

    // the element was found
    if (array[middle] == key)
    {
      pos = middle;
    }

    // try left side
    if (array[middle] > key)
    {
      {
        size_t localPos = binarySearch(array, left, middle - 1, key);
        // return position only if found
        if (localPos != NOT_FOUND)
        {     
          pos = localPos;     
        }
      }
    }// left

    // try right side
    if (array[middle] < key)
    {
      {
        // return position only if found
        size_t localPos = binarySearch(array, middle + 1, right, key);
        if (localPos != NOT_FOUND)
        {     
          pos = localPos;         
        }
      }
    }// right
  } // recursion


  return pos;
}// end of binarySearch
//----------------------------------------------------------------------------------------------------------------------

/**
 * Search in sorted array
 * @param [in] array - Where to search
 * @param [in] key   - What to search.
 * @return Position
 */
template<size_t size>
size_t search(const size_t* array,
              const size_t  key)
{
  //------------------------------------------------------------------------------------------------------------------//
  // 1. Add correct pragmas                                                                                           //
  //    Use pragma taskgroup to enable canceling unnecessary tasks                                                    //
  //------------------------------------------------------------------------------------------------------------------//
  size_t pos = NOT_FOUND;


  pos = binarySearch(array, 0, size - 1, key);

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
  printf(" Parallel Search in sorted array                   \n");
  printf(" Running on: %s\n", hostName);
  printf(" Number of cores: %d\n", omp_get_num_procs());
  #pragma omp parallel
  #pragma omp single
  printf(" Number of threads: %2d\n", omp_get_num_threads());
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
    printf("  - Time to find the key:    %9.3f us\n", 1000 * time / testRept[testId]);

    printf("---------------------------------------------------\n");
  }

  delete [] array;
  return EXIT_SUCCESS;
}// end of main
//----------------------------------------------------------------------------------------------------------------------
