#ifndef PTA_IOCTL_H
#define PTA_IOCTL_H
#include <linux/ioctl.h>
#include <linux/types.h>
#define PTA_MAX_SIZE 65536U
#define PTA_RESIZE _IOW('p', 1, __u32)
#endif
