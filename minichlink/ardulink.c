#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <errno.h>

#include "minichlink.h"

void * TryInit_Ardulink(void);

static int ArdulinkWriteReg32(void * dev, uint8_t reg_7_bit, uint32_t command);
static int ArdulinkReadReg32(void * dev, uint8_t reg_7_bit, uint32_t * commandresp);
static int ArdulinkFlushLLCommands(void * dev);
static int ArdulinkExit(void * dev);
static int ArdulinkTargetReset(void * dev, int reset);


typedef struct {
    struct ProgrammerStructBase psb;
    int fd;
} ardulink_ctx_t;

int ArdulinkWriteReg32(void * dev, uint8_t reg_7_bit, uint32_t command)
{
    uint8_t buf[6];
    buf[0] = 'w';
    buf[1] = reg_7_bit;

    //fprintf(stderr, "WriteReg32: 0x%02x = 0x%08x\n", reg_7_bit, command);

    buf[2] = command & 0xff;
    buf[3] = (command >> 8) & 0xff;
    buf[4] = (command >> 16) & 0xff;
    buf[5] = (command >> 24) & 0xff;

    if (write(((ardulink_ctx_t*)dev)->fd, buf, 6) == -1)
        return -errno;

    if (read(((ardulink_ctx_t*)dev)->fd, buf, 1) == -1)
        return -errno;

    return buf[0] == '+' ? 0 : -EPROTO;
}

int ArdulinkReadReg32(void * dev, uint8_t reg_7_bit, uint32_t * commandresp)
{
    uint8_t buf[4];
    buf[0] = 'r';
    buf[1] = reg_7_bit;

    if (write(((ardulink_ctx_t*)dev)->fd, buf, 2) == -1)
        return -errno;

    if (read(((ardulink_ctx_t*)dev)->fd, buf, 4) == -1)
        return -errno;

    *commandresp = (uint32_t)buf[0] | (uint32_t)buf[1] << 8 | \
        (uint32_t)buf[2] << 16 | (uint32_t)buf[3] << 24;

    //fprintf(stderr, "ReadReg32: 0x%02x = 0x%08x\n", reg_7_bit, *commandresp);

    return 0;
}

int ArdulinkFlushLLCommands(void * dev)
{
    return 0;
}

int ArdulinkExit(void * dev)
{
    close(((ardulink_ctx_t*)dev)->fd);
    free(dev);
    return 0;
}

int ArdulinkTargetReset(void * dev, int reset) {
    char c;

    fprintf(stderr, "Ardulink: target reset %d\n", reset);

    // Assert reset.
    c = reset ? 'a' : 'A';
    if (write(((ardulink_ctx_t*)dev)->fd, &c, 1) == -1)
        return -errno;

    if (read(((ardulink_ctx_t*)dev)->fd, &c, 1) == -1)
        return -errno;

    if (c != '+')
        return -EPROTO;

    usleep(20000);
    return 0;
}

void * TryInit_Ardulink(void)
{
    ardulink_ctx_t *ctx;
    struct termios attr;
    char first;
    int argp = TIOCM_DTR;

    if (!(ctx = calloc(sizeof(ardulink_ctx_t), 1))) {
        perror("calloc");
        return NULL;
    }

    if ((ctx->fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY)) == -1) {
        perror("open");
        return NULL;
    }

    if (tcgetattr(ctx->fd, &attr) == -1) {
        perror("tcgetattr");
        return NULL;
    }

    cfmakeraw(&attr);
    cfsetspeed(&attr, 115200);

    if (tcsetattr(ctx->fd, TCSANOW, &attr) == -1) {
        perror("tcsetattr");
        return NULL;
    }

    // Arduino DTR reset.
    if (ioctl(ctx->fd, TIOCMBIC, &argp) == -1) {
        perror("ioctl");
        return NULL;
    }

    if (tcdrain(ctx->fd) == -1) {
        perror("tcdrain");
        return NULL;
    }

    if (ioctl(ctx->fd, TIOCMBIS, &argp) == -1) {
        perror("ioctl");
        return NULL;
    }

    if (tcdrain(ctx->fd) == -1) {
        perror("tcdrain");
        return NULL;
    }

    // Flush anything that might be in the RX buffer, we need the sync char.
    if (tcflush(ctx->fd, TCIFLUSH) == -1) {
        perror("tcflush");
        return NULL;
    }

    // Let the bootloader do its thing.
    sleep(3);

    if (read(ctx->fd, &first, 1) == -1) {
        perror("read");
        return NULL;
    }

    if (first != '!') {
        fprintf(stderr, "Ardulink: not the sync character.\n");
        return NULL;
    }

    if (ArdulinkTargetReset(ctx, 1) != 0) {
        fprintf(stderr, "Ardulink: target reset failed.\n");
        return NULL;
    }

    fprintf(stderr, "Ardulink: synced.\n");

    MCF.WriteReg32 = ArdulinkWriteReg32;
	MCF.ReadReg32 = ArdulinkReadReg32;
    MCF.FlushLLCommands = ArdulinkFlushLLCommands;
	MCF.Exit = ArdulinkExit;
    MCF.TargetReset = ArdulinkTargetReset;

	return ctx;
}
