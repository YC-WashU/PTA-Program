#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "pta_ioctl.h"

int main(int argc, char **argv)
{
	char *end;
	unsigned long value;
	__u32 size;
	int fd, result;

	if (argc != 2) {
		fprintf(stderr, "usage: resize BYTES\n");
		return 2;
	}
	errno = 0;
	value = strtoul(argv[1], &end, 10);
	if (errno || end == argv[1] || *end || *argv[1] == '-' || value > 0xffffffffUL)
		return 2;
	size = value;
	fd = open("/dev/pta2", O_RDWR);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	result = ioctl(fd, PTA_RESIZE, &size);
	if (result < 0)
		perror("PTA_RESIZE");
	close(fd);
	return result < 0;
}
