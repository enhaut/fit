/**
 * ipk2
 *
 * @file main.c
 *
 * @brief
 *
 * @author Samuel Dobroň (xdobro23), FIT BUT
 *
 */
#include "stdlib.h"

#include "args_parser.h"
#include "devicemanager.h"

#define DEBUG(fmt, ...)                       \
  do {                                        \
    break;fprintf(stderr, fmt "\n", __VA_ARGS__);   \
  } while (0)

void graceful_exit()
{
  free(snifferOptions);  // free(NULL) is safe
  exit(0);
}

/**
 * @brief Entrypoint of sniffer.
 * The main idea of sniffer and what is actually going on is borrowed from
 * https://www.devdungeon.com/content/using-libpcap-c by "NanoDev".
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
  DEBUG("Hi", NULL);
  snifferOptions = process_args(argc, argv);

  DEBUG("Getting devices...", NULL);
  devices_ptr = getDevices();
  DEBUG("Got devices.", NULL);

  if (snifferOptions->inter)
    capture();
  else
    print_device(devices_ptr);

  graceful_exit();
  return 0;
}
