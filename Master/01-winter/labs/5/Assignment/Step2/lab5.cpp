/**
 * @file      lab5.cpp
 *
 * @author    Jiri Jaros \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            jarosjir@fit.vutbr.cz
 *
 * @brief     AVS - PC lab 5
 *            Fibonacci sequence - OpenMP tasks
 *
 * @version   2021
 *
 * @date      21 November  2020, 10:30 (created) \n
 * @date      18 November  2021, 22:07 (revised) \n
 *
 */

#include <unistd.h>
#include <cstdio>
#include <map>
#include <chrono>
#include <functional>
#include <cmath>
#include <omp.h>

using namespace std;

//--------------------------------------------------------------------------------------------------------------------//
//                                     Type definition Function prototypes                                            //
//--------------------------------------------------------------------------------------------------------------------//

/**
 * Fibonacci number search
 */
template<size_t rank>
size_t fibBenchmark();

/// Function pointer definition
using FibBenchmarkFnc = std::function<size_t()>;

//--------------------------------------------------------------------------------------------------------------------//
//                                     Global constants and benchmark maps                                            //
//--------------------------------------------------------------------------------------------------------------------//
// Number of tests
constexpr size_t nTests = 4;
// Size of arrays used
constexpr size_t benchmarkSizes[nTests] = {10, 20, 30, 40};

// Number of benchmark repetitions
constexpr size_t testRept[nTests]       = {1000, 100, 10, 1};

// Max array size
const size_t maxSizes = benchmarkSizes[nTests - 1];

// Map with benchmark sizes
std::map<size_t, FibBenchmarkFnc> fibBenchmarks =
{
  {benchmarkSizes[0], &fibBenchmark<benchmarkSizes[0]>},
  {benchmarkSizes[1], &fibBenchmark<benchmarkSizes[1]>},
  {benchmarkSizes[2], &fibBenchmark<benchmarkSizes[2]>},
  {benchmarkSizes[3], &fibBenchmark<benchmarkSizes[3]>},
};// end of fibBenchmarks

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           Routines to be implemented                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Calculate Fibonacci number for a given number
 * @param   [in] rank - Fibonacci number's rank
 * @return  Fibonacci number
 */
size_t calculateFibNumber(size_t rank)
{
  //------------------------------------------------------------------------------------------------------------------//
  // 2. Use OpenMP tasks parallelize recursive algorithm                                                              //
  // 3. Find a sensible seqThreshold                                                                                  //
  //------------------------------------------------------------------------------------------------------------------//

  size_t x, y;
  constexpr size_t seqThreshold = 16;

  if (rank < 2)
  {
    return rank;
  }


  x = calculateFibNumber(rank - 1);
  y = calculateFibNumber(rank - 2);

  return x + y;
}// end of calculateFibNumber
//----------------------------------------------------------------------------------------------------------------------

/**
 * Fibonacci benchmark
 * @tparam rank - Rank of the Fibonacci number
 * @return Fibonacci number
 */
template<size_t rank>
size_t fibBenchmark()
{
  //------------------------------------------------------------------------------------------------------------------//
  // 1. Use OpenMP tasks to distribute the work                                                                       //
  //------------------------------------------------------------------------------------------------------------------//
  
  return calculateFibNumber(rank);

}// end of fibBenchmark
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
  printf(" Parallel Fibonacci number calculation             \n");
  printf(" Running on: %s\n", hostName);
  printf(" Number of cores:   %2d\n", omp_get_num_procs());
  #pragma omp parallel
  #pragma omp single
  printf(" Number of threads: %2d\n", omp_get_num_threads());
  printf("---------------------------------------------------\n");

  for (size_t testId = 0; testId < nTests; testId++)
  {
    printf("  - Testing pool:             %8d \n", benchmarkSizes[testId]);
    printf("  - Number of repetitions:     %7d\n", testRept[testId]);

    auto startTime = std::chrono::high_resolution_clock::now();

    // Run benchmark
    int fibNumber = 0;
    for (size_t rept = 0; rept < testRept[testId]; rept++)
    {
      #pragma noinline recursive
      fibNumber = fibBenchmarks[benchmarkSizes[testId]]();
    }

    // Elapsed time
    const auto   endTime = std::chrono::high_resolution_clock::now();
    const double time    = (endTime - startTime) / std::chrono::milliseconds(1);

    // Log
    printf("  - Fib number (id %3d):    %10d\n", benchmarkSizes[testId], fibNumber);
    printf("  - Time to compute primes:   %8.3f ms\n", time / testRept[testId]);

    printf("---------------------------------------------------\n");
  }

  return EXIT_SUCCESS;
}// end of main
//----------------------------------------------------------------------------------------------------------------------
