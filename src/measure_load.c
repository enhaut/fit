//
// Created by Samuel Dobron on 14.02.2022.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "measure_load.h"

int measure_load(load_t *load)
{
    char buffer[512];
    FILE *fp;
    if ((fp = fopen("/proc/stat", "r")))
    {
        fread(&buffer, sizeof(char), sizeof(buffer), fp);
        fclose(fp);
        *(strstr(buffer, "\n")) = '\0';  // set first line break as an end of string

        if (sscanf(buffer, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                   &(load->user),
                   &(load->nice),
                   &(load->system),
                   &(load->idle),
                   &(load->iowait),
                   &(load->irq),
                   &(load->softirq),
                   &(load->steal),
                   &(load->guest),
                   &(load->guest_nice)) == 10)
            return 0;
    }
    return 1;
}

int calculate_load()
{
    load_t prev, actual;
    int prev_res = measure_load(&prev);
    sleep(SLEEP_FOR);
    int actual_res = measure_load(&actual);

    if (actual_res || prev_res)
        return -1;

    ULLI previdle, prevnonidle, prevtotal;
    ULLI idle, nonidle, total;
    signed long long totald, idled, perc;

    previdle = prev.idle + prev.iowait;
    idle = actual.idle + actual.iowait;
    prevnonidle = prev.user + prev.nice + prev.system + prev.irq + prev.softirq + prev.steal;
    nonidle = actual.user + actual.nice + actual.system + actual.irq + actual.softirq + actual.steal;
    prevtotal = previdle + prevnonidle;
    total = idle + nonidle;
    totald = total - prevtotal;
    idled = idle - previdle;
    perc = ((totald - idled) * 100)/totald; // *100 to convert it from base of <0, 1> to <0, 100>

    return (int)perc;
}
