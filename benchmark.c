#include <stdio.h>
#include <stdlib.h>
#include "benchmark.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <float.h>

static double convtime(struct timeval *t1, struct timeval *t2)
{
    return (t2->tv_sec-t1->tv_sec) + (t2->tv_usec-t1->tv_usec)*1e-6;
}

int benchmark(int fd, size_t mediumsize, unsigned long sectorsize)
{
    size_t sector = 0;
    double tot_time = 0;
    double t_min, t_max;
    size_t sec_min, sec_max;
    sec_min = sec_max = 0;
    t_min = DBL_MAX;
    t_max = -DBL_MAX;
    struct timeval t1, t2;
    void *buffer = malloc(sectorsize);

    if (buffer == NULL) {
        return -1;
    }

    while (sector < (mediumsize / sectorsize)) {
        gettimeofday(&t1, NULL);
        if (pread(fd, buffer, sectorsize, sector*sectorsize) < 0) {
            printf("Read error at sector: %lu\n", sector);
            perror("pread");
            free(buffer);
            return -2;
        }
        gettimeofday(&t2, NULL);
        double t = convtime(&t1, &t2);
        tot_time += t;
        if (t < t_min) {
            t_min = t;
            sec_min = sector;
        }
        if (t > t_max) {
            t_max = t;
            sec_max = sector;
        }
        ++sector;
    }

    printf("Avg read speed: %.0lf\n", (double) mediumsize/tot_time);
    printf("Peak speed: %.0lf at sector %lu\n", sectorsize/t_min, sec_min);
    printf("Worst speed: %.0lf at sector %lu\n", sectorsize/t_max, sec_max);

    free(buffer);
    return 0;
}
