#define _DEFAULT_SOURCE
#include "pakratt232.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define SOH 0x01
#define DLE 0x10
#define ETB 0x17
#define PK232_COMMAND 0x4f
#define PK232_RAW_DATA 0x20
#define PK232_RAW_RX 0x3f

static struct termios saved_termios;
static int saved_termios_valid;
static unsigned char rx_frame[4096];
static size_t rx_len;
static int rx_in_frame;
static int rx_escape;

static int write_all(int fd, const unsigned char *data, size_t len)
{
    while (len > 0) {
        ssize_t n = write(fd, data, len);
        if (n > 0) {
            data += n;
            len -= (size_t)n;
            continue;
        }
        if (n < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) {
            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) usleep(1000);
            continue;
        }
        return -1;
    }
    return 0;
}

static int read_for(int fd, unsigned char *out, size_t capacity,
                    unsigned timeout_ms)
{
    size_t len = 0;
    struct timeval start, now;
    gettimeofday(&start, NULL);
    while (len < capacity) {
        gettimeofday(&now, NULL);
        uint64_t elapsed = (uint64_t)(now.tv_sec - start.tv_sec) * 1000000ULL;
        if (now.tv_usec >= start.tv_usec) elapsed += (uint64_t)(now.tv_usec - start.tv_usec);
        else elapsed -= (uint64_t)(start.tv_usec - now.tv_usec);
        if (elapsed >= (uint64_t)timeout_ms * 1000ULL) break;
        uint64_t remaining = (uint64_t)timeout_ms * 1000ULL - elapsed;
        struct timeval tv = {
            (time_t)(remaining / 1000000ULL),
            (suseconds_t)(remaining % 1000000ULL)
        };
        fd_set set;
        FD_ZERO(&set);
        FD_SET(fd, &set);
        int rc = select(fd + 1, &set, NULL, NULL, &tv);
        if (rc == 0) break;
        if (rc < 0) {
            if (errno == EINTR) continue;
            break;
        }
        ssize_t n = read(fd, out + len, capacity - len);
        if (n > 0) len += (size_t)n;
        else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) continue;
        else break;
    }
    return (int)len;
}

static int send_host_block(int fd, unsigned char ctl,
                           const unsigned char *payload, size_t len)
{
    unsigned char frame[8192];
    size_t pos = 0;
    frame[pos++] = SOH;
    frame[pos++] = ctl;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = payload[i];
        if (c == SOH || c == DLE || c == ETB) frame[pos++] = DLE;
        if (pos >= sizeof(frame) - 2) return -1;
        frame[pos++] = c;
    }
    frame[pos++] = ETB;
    return write_all(fd, frame, pos);
}

int pakratt232_open(const char *device, speed_t speed)
{
    int fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) return -1;
    if (tcgetattr(fd, &saved_termios) < 0) { close(fd); return -1; }
    saved_termios_valid = 1;

    struct termios tio = saved_termios;
    cfmakeraw(&tio);
    tio.c_cflag &= ~(CSIZE | PARENB | PARODD | CSTOPB | HUPCL);
    tio.c_cflag |= CS8 | CREAD | CLOCAL;
#ifdef CRTSCTS
    tio.c_cflag |= CRTSCTS;
#endif
    cfsetispeed(&tio, speed);
    cfsetospeed(&tio, speed);
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &tio) < 0) { close(fd); return -1; }

#ifdef TIOCMGET
    int bits = 0;
    if (ioctl(fd, TIOCMGET, &bits) == 0) {
#ifdef TIOCM_DTR
        bits |= TIOCM_DTR;
#endif
#ifdef TIOCM_RTS
        bits |= TIOCM_RTS;
#endif
#ifdef TIOCMSET
        (void)ioctl(fd, TIOCMSET, &bits);
#endif
    }
#endif
    tcflush(fd, TCIOFLUSH);
    rx_len = 0;
    rx_in_frame = 0;
    rx_escape = 0;
    return fd;
}

int pakratt232_init(int fd)
{
    static const unsigned char entry[] =
        {0x11, 0x18, 0x03, 'H', 'O', 'S', 'T', ' ', 'Y', 0x0d};
    static const unsigned char soh = SOH;
    static const unsigned char sync[] = {SOH, PK232_COMMAND, 'G', 'G', ETB};
    unsigned char rx[256];

    if (write_all(fd, entry, sizeof(entry)) < 0) return -1;
    usleep(300000);
    (void)read_for(fd, rx, sizeof(rx), 100);
    if (write_all(fd, &soh, 1) < 0 || write_all(fd, sync, sizeof(sync)) < 0) return -1;
    int n = read_for(fd, rx, sizeof(rx), 1200);
    for (int i = 0; i + 5 < n; ++i) {
        if (rx[i] == SOH && rx[i + 1] == PK232_COMMAND &&
            rx[i + 2] == 'G' && rx[i + 3] == 'G' &&
            rx[i + 4] == 0x00 && rx[i + 5] == ETB) {
            static const unsigned char rw[] = "RW ON";
            static const unsigned char ki[] = "KI OFF";
            (void)send_host_block(fd, PK232_COMMAND, rw, sizeof(rw) - 1);
            (void)send_host_block(fd, PK232_COMMAND, ki, sizeof(ki) - 1);
            return 0;
        }
    }
    return -1;
}

int pakratt232_send_raw(int fd, const unsigned char *data, size_t len)
{
    return send_host_block(fd, PK232_RAW_DATA, data, len);
}

int pakratt232_read_frame(int fd, unsigned char *ctl, unsigned char *data,
                          size_t capacity, size_t *len)
{
    unsigned char input[1024];
    ssize_t n;
    *len = 0;
    for (;;) {
        n = read(fd, input, sizeof(input));
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) break;
        if (n <= 0) break;
        for (ssize_t i = 0; i < n; ++i) {
            unsigned char c = input[i];
            if (!rx_in_frame) {
                if (c == SOH) { rx_in_frame = 1; rx_escape = 0; rx_len = 0; }
                continue;
            }
            if (rx_len == 0) { rx_frame[rx_len++] = c; continue; }
            if (rx_escape) { rx_escape = 0; if (rx_len < sizeof(rx_frame)) rx_frame[rx_len++] = c; continue; }
            if (c == DLE) { rx_escape = 1; continue; }
            if (c == ETB) {
                rx_in_frame = 0;
                if (rx_len < 1) continue;
                *ctl = rx_frame[0];
                *len = rx_len - 1;
                if (*len > capacity) { rx_len = 0; return -1; }
                memcpy(data, rx_frame + 1, *len);
                rx_len = 0;
                return 1;
            }
            if (rx_len < sizeof(rx_frame)) rx_frame[rx_len++] = c;
            else { rx_in_frame = 0; rx_len = 0; }
        }
    }
    return 0;
}

void pakratt232_close(int fd)
{
    if (saved_termios_valid) (void)tcsetattr(fd, TCSANOW, &saved_termios);
    saved_termios_valid = 0;
    close(fd);
}
