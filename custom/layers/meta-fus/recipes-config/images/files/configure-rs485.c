#include <linux/serial.h>
#include <sys/ioctl.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    int fd = open("/dev/ttymxc3", O_RDWR);
    if (fd < 0) {}

    struct serial_rs485 rs485conf;

    rs485conf.flags = SER_RS485_ENABLED | SER_RS485_RTS_ON_SEND | SER_RS485_RX_DURING_TX;

    int err = ioctl(fd, TIOCSRS485, &rs485conf);
    if (err) {
        printf("%s: Unable to configure port in 485 mode, status (%i)\n", argv[1], err);
        return -1;
    }

    // if (close (fd) < 0) {}
    
    return 0;
}
