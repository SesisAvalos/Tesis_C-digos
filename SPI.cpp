//Con este código se configura y controla la interfáz gráfica SPI (Serial Peripheral Interface) en la Raspberry
//Se agrega SPI.h
#include "SPI.h"

//Se le agregan valores a las variables declaradas en spi.h
int spi_cs0_fd = -1;
int spi_cs1_fd = -1;

unsigned char spi_mode = SPI_MODE_3;
unsigned char spi_bitsPerWord = 8;
unsigned int spi_speed = 10000000;

//Se abre y configura un puerto SPI
int SpiOpenPort (int spi_device, unsigned int useSpiSpeed)
{
	int status_value = -1;
	int *spi_cs_fd;

//Se configuran los parámetros de SPI
	spi_mode = SPI_MODE_3;
	spi_bitsPerWord = 8;			//Bits por palabra
	spi_speed = useSpiSpeed;		// Bus Speed 1000000 = 1MHz (1uS per bit)

//Se asigna un puntero el file descriptor correspondiente según el tipo
	if (spi_device)
		spi_cs_fd = &spi_cs1_fd;
	else
		spi_cs_fd = &spi_cs0_fd;

//Se abre el dispositivo SPI en modo de lectura o escritura
	if (spi_device)
		*spi_cs_fd = open(std::string("/dev/spidev0.1").c_str(), O_RDWR);
	else
		*spi_cs_fd = open(std::string("/dev/spidev0.0").c_str(), O_RDWR);


//Se verifica y open() falló
	if (*spi_cs_fd < 0)
	{
		perror("Error - Could not open SPI device");
		exit(1);
	}

//Se configuran parámetros del dispositivo SPI a través de ioctl
	
	//Escribe el modo SPI
	status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MODE, &spi_mode);
	if(status_value < 0)
	{
		perror("Could not set SPIMode (WR)...ioctl fail");
		exit(1);
	}

//Se lee el SPI para corroborar que se haya configurado correctamente
	status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MODE, &spi_mode);
	if(status_value < 0)
	{
		perror("Could not set SPIMode (RD)...ioctl fail");
		exit(1);
	}

//Escribe bits por palabra
	status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bitsPerWord);
	if(status_value < 0)
	{
		perror("Could not set SPI bitsPerWord (WR)...ioctl fail");
		exit(1);
	}

//Lee bits por palabras
	status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_BITS_PER_WORD, &spi_bitsPerWord);
	if(status_value < 0)
	{
		perror("Could not set SPI bitsPerWord(RD)...ioctl fail");
		exit(1);
	}

//Escribe y luego lee el número de bits por palabra
	status_value = ioctl(*spi_cs_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
	if(status_value < 0)
	{
		perror("Could not set SPI speed (WR)...ioctl fail");
		exit(1);
	}

	//Escribe y luego lee la velocidad máxima del bus SPI 
	status_value = ioctl(*spi_cs_fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
	if(status_value < 0)
	{
		perror("Could not set SPI speed (RD)...ioctl fail");
		exit(1);
	}
	return(status_value);
}

//Función para cerrar el puerto SPI
int SpiClosePort(int spi_device)
{
		int status_value = -1;
	int *spi_cs_fd;

	if (spi_device)
		spi_cs_fd = &spi_cs1_fd;
	else
		spi_cs_fd = &spi_cs0_fd;


	status_value = close(*spi_cs_fd);
	if(status_value < 0)
	{
		perror("Error - Could not close SPI device");
		exit(1);
	}
	return(status_value);
}
