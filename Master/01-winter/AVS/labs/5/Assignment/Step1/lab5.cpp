/**
 * @file      lab5.cpp
 *
 * @author    Jiri Jaros \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            jarosjir@fit.vutbr.cz
 *
 * @brief     AVS - PC lab 5
 *            Sieve of Eratosthenes - OpenMP tasks
 *
 * @version   2021
 *
 * @date      20 November  2020, 19:22 (created) \n
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
 * Sieve of Eratosthenes
 */
template<size_t size>
int sieve();

/// Function pointer definition
using SieveBenchmarkFnc = std::function<int()>;

//--------------------------------------------------------------------------------------------------------------------//
//                                     Global constants and benchmark maps                                            //
//--------------------------------------------------------------------------------------------------------------------//
// Number of tests
constexpr size_t nTests = 4;
// Size of arrays used
constexpr size_t benchmarkSizes[nTests] = {1000, 10000, 100000, 10000000};

// Number of benchmark repetitions
constexpr size_t testRept[nTests]       = {1000, 1000, 500, 3};

// Max array size
const size_t maxSizes = benchmarkSizes[nTests - 1];

// Map with benchmark sizes
std::map<size_t, SieveBenchmarkFnc> sieveBenchmarks =
{
  {benchmarkSizes[0], &sieve<benchmarkSizes[0]>},
  {benchmarkSizes[1], &sieve<benchmarkSizes[1]>},
  {benchmarkSizes[2], &sieve<benchmarkSizes[2]>},
  {benchmarkSizes[3], &sieve<benchmarkSizes[3]>},
};// end of sieveBenchmarks

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                           Routines to be implemented                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int getNumberOfPrimes(int low, int high)
{
  int nPrimes = 0;

  for (int number = low; number < high; number++)
  {
    bool       isPrime    = true;
    const int  maxDivisor = sqrt(number) + 1;

    // Test all divisors
    for (int divisor = 2; divisor < maxDivisor; divisor++)
    {
      if (number % divisor == 0)
      {
        isPrime = false;
        break;
      }
    }

    if (isPrime)
    {
      nPrimes++;
      isPrime = true;
    }
  }

  return nPrimes;
}// end of getNumberOfPrimes
//----------------------------------------------------------------------------------------------------------------------


/**
 * Sieve of Eratosthenes
 * @tparam size - Size of the pool to test
 * @return Number of primes
 */
template<size_t size>
int sieve()
{
  //------------------------------------------------------------------------------------------------------------------//
  // 1. Use OpenMP tasks to distribute the work                                                                       //
  // 2. Find a suitable chunk size                                                                                    //
  // 3. Be careful when updating nPrimes                                                                              //
  //------------------------------------------------------------------------------------------------------------------//

  constexpr int chunkSize = 8000;
  int nPrimes = 0;

  #pragma omp parallel
  {
    #pragma omp single nowait
    for (int number = 1; number <= size; number += chunkSize)
    {
       #pragma omp task
       {
         #pragma omp atomic update
         nPrimes += getNumberOfPrimes(number, ((number + chunkSize) > size) ? size : number + chunkSize);
       }
    }
  }

  return nPrimes;
}// end of sieve
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
  printf(" Parallel Sieve of Eratosthenes                    \n");
  printf(" Running on: %s\n", hostName);
  printf(" Number of cores:   %2d\n", omp_get_num_procs());
  #pragma omp parallel
  #pragma omp single
  printf(" Number of threads: %2d\n", omp_get_num_threads());
  printf("---------------------------------------------------\n");

  for (size_t testId = 0; testId < nTests; testId++)
  {
    printf("  - Testing pool:           %8d \n", benchmarkSizes[testId]);
    printf("  - Number of repetitions:   %7d\n", testRept[testId]);

    auto startTime = std::chrono::high_resolution_clock::now();

    // Run benchmark
    int nPrimes = 0;
    for (size_t rept = 0; rept < testRept[testId]; rept++)
    {
      #pragma noinline recursive
      nPrimes = sieveBenchmarks[benchmarkSizes[testId]]();
    }

    // Elapsed time
    const auto   endTime = std::chrono::high_resolution_clock::now();
    const double time    = (endTime - startTime) / std::chrono::milliseconds(1);

    // Log
    printf("  - Number of primes found:  %7d\n", nPrimes);
    printf("  - Time to compute primes: %8.3f ms\n", time / testRept[testId]);

    printf("---------------------------------------------------\n");
  }

  return EXIT_SUCCESS;
}// end of main
//----------------------------------------------------------------------------------------------------------------------