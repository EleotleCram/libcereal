/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//   This file is largely based on stuff found here:                                   //
//                                                                                     //
//     http://todbot.com/blog/2006/12/06/arduino-serial-c-code-to-talk-to-arduino/     //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////


#include "../Serial.h"

#ifdef __unix__

#include <stdio.h>    /* Standard input/output definitions */
#include <stdlib.h>
#include <stdint.h>   /* Standard types */
#include <string.h>   /* String function definitions */
#include <unistd.h>   /* UNIX standard function definitions */
#include <fcntl.h>    /* File control definitions */
#include <errno.h>    /* Error number definitions */
#include <termios.h>  /* POSIX terminal control definitions */
#include <sys/ioctl.h>

int Baud2TermiosOption(Serial::Baud baud) {
	speed_t termiosBaud = -1;
	switch(baud) {
		case 4800:   termiosBaud=B4800;   break;
		case 9600:   termiosBaud=B9600;   break;
	#ifdef B14400
		case 14400:  termiosBaud=B14400;  break;
	#endif
		case 19200:  termiosBaud=B19200;  break;
	#ifdef B28800
		case 28800:  termiosBaud=B28800;  break;
	#endif
		case 38400:  termiosBaud=B38400;  break;
		case 57600:  termiosBaud=B57600;  break;
		case 115200: termiosBaud=B115200; break;
	}
	return termiosBaud;
}

Serial::Serial(const char *portName, Baud baud) {
	this->connected = false;

	struct termios toptions;

	this->fd = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);
	if(this->fd == -1) {
		perror("Serial: Unable to open port ");
	} else {
		speed_t termiosBaud = Baud2TermiosOption(baud);
		if(termiosBaud < 0) {
			fprintf(stderr, "Serial: Unable to set baud rate: %d\n", baud);
		} else {
			if(tcgetattr(this->fd, &toptions) < 0) {
				perror("Serial: Couldn't get term attributes");
			}
			cfsetispeed(&toptions, termiosBaud);
			cfsetospeed(&toptions, termiosBaud);

			// 8N1
			toptions.c_cflag &= ~PARENB;
			toptions.c_cflag &= ~CSTOPB;
			toptions.c_cflag &= ~CSIZE;
			toptions.c_cflag |= CS8;
			// no flow control
			toptions.c_cflag &= ~CRTSCTS;

			toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
			toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

			toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
			toptions.c_oflag &= ~OPOST; // make raw

			// see: http://unixwiz.net/techtips/termios-vmin-vtime.html
			toptions.c_cc[VMIN]  = 0;
			toptions.c_cc[VTIME] = 20;

			if( tcsetattr(this->fd, TCSANOW, &toptions) < 0) {
				perror("Serial: Couldn't set term attributes");
			}

			this->connected = true;
		}
	}
}

Serial::~Serial() {
	if(this->connected) {
		this->connected = false;
		close(this->fd);
	}
}

static inline int posix_read(int fd, char* buffer, int size) {
    return read(fd, buffer, size);
}
int Serial::read(char *buffer, unsigned int size) {
	return posix_read(this->fd, buffer, size);
}

static inline int posix_write(int fd, char* buffer, int size) {
    return write(fd, buffer, size);
}
int Serial::write(char *buffer, unsigned int size) {
	return posix_write(this->fd, buffer, size);
}

#endif
