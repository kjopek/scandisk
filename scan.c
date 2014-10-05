#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/disk.h>

#include <unistd.h>
#include <fcntl.h>

#include "scan.h"

static void fill_pattern_1(void* buffer, unsigned long sectorsize)
{
    unsigned long i = 0;
    uint8_t *ptr = (uint8_t*) buffer;
    while (i<sectorsize) {
        ptr[i] = (i % 2 == 0) ? 0xff : 0x00;
        ++i;
    }
}

static int compare_pattern_1(void *buffer, unsigned long sectorsize)
{
    unsigned long i = 0;
    uint8_t *ptr = (uint8_t*) buffer;
    while (i<sectorsize) {
        if (ptr[i] != ((i % 2 == 0) ? 0xff : 0x00)) {
            return -1;
        }
        ++i;
    }
    return 0;
}

int scan_disk(int fd, size_t mediumsize, unsigned long sectorsize)
{
    void *orig_buffer = malloc(sectorsize);
    void *buffer = malloc(sectorsize);
    size_t sector = 0;

    if (orig_buffer == NULL) {
        fprintf(stderr, "Cannot allocate memory for orig_buffer");
        return -1;
    }

    if (buffer == NULL) {
        fprintf(stderr, "Cannot allocate memory for buffer");
        free(orig_buffer);
        return -1;
    }

    while (sector < (mediumsize/sectorsize)) {
        fprintf(stderr, "\rSector: %lu / %lu", sector, mediumsize/sectorsize);
        if (pread(fd, orig_buffer, sectorsize, sector*sectorsize) < 0) {
            fprintf(stderr, "Read error at sector: %lu\n", sector);
            perror("pread");
            free(orig_buffer);
            free(buffer);
            return -2;
        }

        fill_pattern_1(buffer, sectorsize);

        if (pwrite(fd, buffer, sectorsize, sector*sectorsize) < 0) {
            fprintf(stderr, "Write error at sector: %lu\n", sector);
            perror("pwrite");
        }

        fsync(fd);

        if (pread(fd, buffer, sectorsize, sector*sectorsize) < 0) {
            fprintf(stderr, "Read error at sector: %lu\n", sector);
            perror("pread");
        }

        if (compare_pattern_1(buffer, sectorsize) < 0) {
            fprintf(stderr, "Incosistency at sector: %lu\n", sector);
        }

        if (pwrite(fd, orig_buffer, sectorsize, sector*sectorsize) < 0) {
            fprintf(stderr,"Write error at sector: %lu\n", sector);
            perror("pwrite");
            free(orig_buffer);
            free(buffer);
            return -2;
        }
        fsync(fd);
        ++sector;
    }

    free(orig_buffer);
    free(buffer);
    return 0;
}
