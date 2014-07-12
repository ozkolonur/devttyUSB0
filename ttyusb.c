#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <string.h>
#include "hexdump.h"

#define TRACE printf("%d:%s:\n", __LINE__, __func__)
//#define printd(args...) printf(args...)
#define printd(args...)

int get_fwversion(int fd, char *fwversion)
{
	int numbytes = -1;

	char cmd[] = {0x80, 0x01, 0xFE, 0x0A, 0x81, 0xF4};
	char buf[64] = {0};

	usleep(100000);
	numbytes = write(fd, cmd, sizeof(cmd));

	if (numbytes < 0)
	{
		perror(__func__);
		return -1;
	}
	else
		printd("command send [%s]\n", __func__);

	usleep(100000);
	numbytes = read(fd, &buf, 64);

	if ((numbytes < 0) || (buf[0] != -128))
	{
		perror(__func__);
		return -2;
	}

	sprintf(fwversion, "%d.%02d", buf[4], buf[5]);	

	return 0;
}

int get_numresult(int fd, int *numresult)
{

	int numbytes = -1;

	char cmd[] = {0x80, 0x01, 0xFE, 0x00, 0x81, 0xFE};
	char buf[64] = {0};

	usleep(100000);
	numbytes = write(fd, cmd, sizeof(cmd));

	if (numbytes < 0)
	{
		perror(__func__);
		return -1;
	}
	else
		printd("command send [%s]\n", __func__);

	usleep(100000);
	numbytes = read(fd, &buf, 64);
	if ((numbytes < 0) || (buf[0] != -128))
	{
		perror(__func__);
		return -2;
	}


	*numresult = (int)buf[4];

	return 0;
}

int get_meterid(int fd, char *meterid)
{
	int numbytes = -1;

	char cmd[] = {0x80, 0x01, 0xFE, 0x09, 0x81, 0xF7};
	char buf[64] = {0};
	char *bufptr = buf + 4;
	usleep(100000);
	numbytes = write(fd, cmd, sizeof(cmd));

	if (numbytes < 0)
	{
		perror(__func__);
		return -1;
	}
	else
		printd("command send [%s]\n", __func__);

	usleep(100000);
	numbytes = read(fd, &buf, 64);

	if ((numbytes < 0) || (buf[0] != -128))
	{
		perror(__func__);
		return -2;
	}
	
	strncpy(meterid, bufptr, 10);
	meterid[10] = '\0';

	return 0;
}

int get_measurement(int fd, int num, char *measurement, char *date_time)
{
	int numbytes = -1;
	int tmp= 0;
	int year, day, month, temp, hour, min;

	char cmd[] = {0x80, 0x02, 0xFD, 0x01, 0x00, 0x82, 0xFC};
	char buf[64] = {0};

	if ((num >= 1) && (num <= 255))
		cmd[4] = (char)num - 1;
	else
		return -1;

	numbytes = write(fd, cmd, sizeof(cmd));

	if (numbytes < 0)
	{
		perror(__func__);
		return -1;
	}
	else
		printd("command send [%s]\n", __func__);

	usleep(100000);
	numbytes = read(fd, &buf, 64);

	if ((numbytes < 0) || (buf[0] != -128))
	{
		perror(__func__);
		return -2;
	}

	sprintf(measurement, "%d", (int)buf[8]);	
	
	tmp = (int)buf[5];
	tmp = tmp << 8;
	tmp = tmp + (int)buf[6];
	
	day = tmp & (0b11111);
	month = (tmp & ((0b1111) << 5)) >> 5;
	year = 2000 + ( ( tmp & ((0b1111111) << 9)) >> 9);

//	printf("%d\n", day);
//	printf("%d\n", month);
//	printf("%d\n", year);

	tmp = 0;
	tmp = (int)buf[9];
	tmp = 1 + tmp << 8;
	tmp = tmp + (int)buf[10];
	
	min = tmp & (0b111111);
	hour = ( tmp & ( (0b11111) << 6 )) >> 6; 
	
	sprintf(date_time, "%d/%d/%d %d:%d", day, month, year, hour, min);

//	printf("date_time=%s", date_time);

	return 0;
}


int main(int argc, char *argv[])
{
	int fd, numbytes;
    char buf[1024] = { 0 };
	char tosend[] = {0x80, 0x01, 0xFE, 0x00, 0x81, 0xFE};
	struct termios options;
	fd = open("/dev/ttyUSB1", O_RDWR | O_NOCTTY | O_NDELAY);
	char fwversion[8];
	char meterid[16];
	char meas[8];
	char date_time[32];
	int numresult;
	int res;
fcntl(fd, F_SETFL, FNDELAY);                  /* Configure port reading */
                                     /* Get the current options for the port */
 tcgetattr(fd, &options);
 cfsetispeed(&options, B9600);                 /* Set the baud rates to 9600 */
 cfsetospeed(&options, B9600);
    
                                   /* Enable the receiver and set local mode */
 options.c_cflag |= (CLOCAL | CREAD);
 options.c_cflag &= ~PARENB; /* Mask the character size to 8 bits, no parity */
 options.c_cflag &= ~CSTOPB;
 options.c_cflag &= ~CSIZE;
 options.c_cflag |=  CS8;                              /* Select 8 data bits */
 options.c_cflag &= ~CRTSCTS;               /* Disable hardware flow control */  

                                 /* Enable data to be processed as raw input */
 options.c_lflag &= ~(ICANON | ECHO | ISIG);
       
                                        /* Set the new options for the port */
 tcsetattr(fd, TCSANOW, &options);

	res = get_fwversion(fd, (char *)&fwversion);
	if (res == 0)
		printf("fwversion=%s\n", fwversion);

	res = get_numresult(fd, &numresult);
	if (res == 0)
		printf("numresult=%d\n", numresult);

	res = get_meterid(fd, (char *)&meterid);
	if (res == 0)
		printf("meterid=%s\n", meterid);

	res = get_measurement(fd, 1, (char *)&meas, (char *)&date_time);
	if (res == 0){
		printf("meas 1 = %s\n", meas);
		printf("date_time = %s\n", date_time);
	}

	close(fd);

	return 0;

}
