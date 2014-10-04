/////////////////////////////////////////////////////////////////
//                                                             //
//   This file is largely based on stuff found here:           //
//                                                             //
//     http://playground.arduino.cc/Interfacing/CPPWindows     //
//                                                             //
/////////////////////////////////////////////////////////////////

#pragma once

#define ARDUINO_WAIT_TIME 2000

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>

class Serial
{
	private:
		/** Connection status */
		bool connected;

#ifdef WIN32
		/** Serial communication handle */
		HANDLE handle;
		/** Get various information about the connection */
		COMSTAT status;
		/** Keep track of last error */
		DWORD errors;
#else
		/** file descriptor */
		int fd;
#endif

    public:
		/**
		 * Baud rate options
		 */
		enum Baud {
			BR_110    = 110,
			BR_300    = 300,
			BR_600    = 600,
			BR_1200   = 1200,
			BR_2400   = 2400,
			BR_4800   = 4800,
			BR_9600   = 9600,
			BR_14400  = 14400,
			BR_19200  = 19200,
			BR_38400  = 38400,
			BR_56000  = 56000,
			BR_57600  = 57600,
			BR_115200 = 115200,
			BR_128000 = 128000,
			BR_256000 = 256000,
		};

		/**
		 * Initialize serial communication with the given port
		 * (COMx, friendly-name, device name, /dev/ttyUSBx, etc.)
		 * and, optionally, given baud rate.
		 *
		 * Communication is fixed to 8N1 (8-bits; no parity; 1 stop bit),
		 * i.e. suits 99.999% of serial communication.
		 */
		Serial(const char *portName, Baud baud=BR_115200);
		/**
		 * Terminate the serial communication
		 */
		~Serial();
		/**
		 * Read at most size bytes into buffer, returns the number
		 * of bytes actually read
		 */
		int read(char *buffer, unsigned int size);
		/**
		 * Write at most size bytes from buffer, returns the number
		 * of bytes actually written
		 */
		int write(char *buffer, unsigned int size);
		/**
		 * Returns whether the serial port is connected
		 */
		bool isConnected();
};
