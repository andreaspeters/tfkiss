#ifndef TFKISS_PAKRATT232_H
#define TFKISS_PAKRATT232_H

#include <stddef.h>
#include <termios.h>

/* AEA PK-232 host-mode backend. */
int pakratt232_open(const char *device, speed_t speed);
int pakratt232_init(int fd);
int pakratt232_send_raw(int fd, const unsigned char *data, size_t len);
int pakratt232_read_frame(int fd, unsigned char *ctl, unsigned char *data,
                          size_t capacity, size_t *len);
void pakratt232_close(int fd);

#endif
