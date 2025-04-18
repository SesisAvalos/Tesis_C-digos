//Se incluyen todas las librerías necesarias
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

//Se declaran variables externas en C++
extern int spi_cs0_fd;
extern int spi_cs1_fd;
extern unsigned char spi_mode;
extern unsigned char spi_bitsPerWord;
extern unsigned int spi_speed;

//Se crean dos funciones, una para abrir un puerto SPI y otra para cerrarlo
int SpiOpenPort(int spi_device, unsigned int spi_speed);
int SpiClosePort(int spi_device);

#endif
