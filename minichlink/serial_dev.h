#ifndef _SERIAL_DEV_H
#define _SERIAL_DEV_H

#include <stddef.h>

#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define IS_WINDOWS
#define DEFAULT_SERIAL_NAME "\\\\.\\COM3"
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#define IS_POSIX
#define DEFAULT_SERIAL_NAME "/dev/ttyACM0"
#endif
/* these are available on all platforms */
#include <errno.h>
#include <stdio.h>

typedef struct {
    const char* port;
    unsigned baud;
#ifdef IS_WINDOWS
    HANDLE handle;
#else
    int fd;
#endif
} serial_dev_t;

/* returns 0 if OK */
int serial_dev_create(serial_dev_t *dev, const char* port, unsigned baud);
/* returns 0 if OK */
int serial_dev_open(serial_dev_t *dev);
/* returns -1 on write error */
int serial_dev_write(serial_dev_t *dev, const void* data, size_t len);
/* returns -1 on read error */
int serial_dev_read(serial_dev_t *dev, void* data, size_t len);
/* returns -1 on reset error */
int serial_dev_do_dtr_reset(serial_dev_t *dev);
/* returns -1 on flush error */
int serial_dev_flush_rx(serial_dev_t *dev);
/* returns -1 on close error */
int serial_dev_close(serial_dev_t *dev);

#endif
