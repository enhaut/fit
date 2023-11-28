/**
 * @file      lab4.cpp
 *
 * @author    Jiri Jaros \n
 *            Faculty of Information Technology \n
 *            Brno University of Technology \n
 *            jarosjir@fit.vutbr.cz
 *
 * @brief     AVS - PC lab 4 - Parallelization with OpenMP
 *            Hello world
 *
 * @version   2021
 *
 * @date      08 November  2020, 18:45 (created) \n
 * @date      05 November  2021, 11:42 (revised) \n
 *
 */

#include <unistd.h>
#include <cstdio>
#include <omp.h>

using namespace std;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                       Function to be implemented                                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Parallel Hello world
 * @param [in] numOfThreads - Number of threads.
 */
void parallelHelloWorld(int numOfThreads)
{
  //------------------------------------------------------------------------------------------------------------------//
  // 1. Find out the thread id and number of threads in the seq. region                                               //
  //------------------------------------------------------------------------------------------------------------------//

  int threadId  = -1;
  int threadNum = 0;

  //------------------------------------------------------------------------------------------------------------------//
  // 2. Run this block by a given number of threads (numOfThreads)                                                    //
  //------------------------------------------------------------------------------------------------------------------//
  {
    //----------------------------------------------------------------------------------------------------------------//
    // 3. Find out the thread id and number of threads in the par. region                                             //
    //----------------------------------------------------------------------------------------------------------------//
    threadId  = -1;
    threadNum = 0;

    printf(" - Hi from parallel region. I'm %d of %d.\n", threadId, threadNum);
  }

  // Back in the sequential region
  fflush(stdout);
  printf("---------------------------------------------------\n");
  printf(" - Hi from serial region. I'm %d of %d.\n", threadId, threadNum);
  // seq part
}// end of parallelHelloWorld
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
  printf(" Parallelization with OpenMP                       \n");
  printf(" Running on: %s\n", hostName);
  printf(" Number of cores: %d\n", omp_get_num_procs());
  printf("---------------------------------------------------\n");

  //------------------------------------------------------------------------------------------------------------------//
  // Test: Hello world                                                                                                //
  //------------------------------------------------------------------------------------------------------------------//
  printf(" - Test 1: Parallel Hello world\n");

  parallelHelloWorld(20);

  printf("---------------------------------------------------\n");

  return EXIT_SUCCESS;
}// end of main
//----------------------------------------------------------------------------------------------------------------------

