#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/disk.h>
#include <sys/ioctl.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

#include "benchmark.h"
#include "scan.h"

static struct option longopts[] = {
    {"device",     required_argument,      NULL,           'd'},
    {"help",       no_argument,            NULL,           'h'},
    {NULL,         0,                      NULL,           0}
};

void usage(void)
{
    printf("Usage: scandisk\n");
}

int main(int argc, char ** argv)
{
    size_t blocksize = 512;
    size_t mediasize = 0;
    unsigned int sectorsize = 0;

    char devname[255] = "";
    int ch;
    int fd = -1;
    struct stat dev_stat;

    while ((ch = getopt_long(argc, argv, "d:h", longopts, NULL)) != -1) {
        switch (ch) {
            case 'd':
                strncpy(devname, optarg, sizeof(devname)-1);
                break;
            case 'h':
            default:
                usage();
                return 0;
        }
    }

    if ( (fd = open(devname, O_RDWR | O_DIRECT | O_SYNC)) < 0) {
        perror("open");
        return 1;
    }

    fprintf(stderr, "Scandisk (c) kj 2014\n");
    fprintf(stderr, "Scanning device: %s\n", devname);

    fstat(fd, &dev_stat);
    blocksize = dev_stat.st_blksize;
    if (ioctl(fd, DIOCGMEDIASIZE, &mediasize)) {
        perror("ioctl");
        close(fd);
        return 1;
    }

    if (ioctl(fd, DIOCGSECTORSIZE, &sectorsize)) {
        perror("ioctl");
        close(fd);
        return 1;
    }

    fprintf(stderr, "\tdevice size: %lu\n", mediasize);
    fprintf(stderr, "\tsector size: %u\n", sectorsize);

    //benchmark(fd, mediasize, blocksize);

    fprintf(stderr, "starting scan\n");

    if (scan_disk(fd, mediasize, blocksize)) {
        fprintf(stderr, "scan error!\n");
    }

    close(fd);
    return 0;
}
