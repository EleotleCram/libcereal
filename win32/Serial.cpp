/////////////////////////////////////////////////////////////////
//                                                             //
//   This file is largely based on stuff found here:           //
//                                                             //
//     http://playground.arduino.cc/Interfacing/CPPWindows     //
//                                                             //
/////////////////////////////////////////////////////////////////

#ifdef WIN32
#include "targetver.h"
#define _AFXDLL
#include <afx.h>
#endif

#include <memory>
#include <string>

#include "../Serial.h"

#ifdef WIN32

#include "EnumSerial.h"

static std::unique_ptr<std::string> GetPortName(const char* friendlyName) {
	std::unique_ptr<std::string> portName;
	CArray<SSerInfo,SSerInfo&> asi;

	// Populate the list of serial ports.
	EnumSerialPorts(asi,FALSE/*include all*/);
	for (int ii=0; ii<asi.GetSize(); ii++) {
		//printf("friendly: |%s|%s|%s|\n", asi[ii].strFriendlyName, asi[ii].strPortName, asi[ii].strPortDesc);
		if(!strcmp(asi[ii].strPortDesc, friendlyName)) {
			portName.reset(new std::string(asi[ii].strPortName));
		}
	}

	return portName;
}

Serial::Serial(const char *portName, Baud baud) {
	//We're not yet connected
	this->connected = false;

	std::unique_ptr<std::string> portNamePtr;
	if(strncmp(portName, "COM", 3)) {
		// Find COM port from friendlyName
		portNamePtr = GetPortName(portName);
		if(portNamePtr) {
			portName = portNamePtr->c_str();
		}
	}

	//Try to connect to the given port through CreateFile
	this->handle = CreateFileA(portName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

	//Check if the connection was successfull
	if(this->handle==INVALID_HANDLE_VALUE) {
		//If not success full display an Error
		if(GetLastError()==ERROR_FILE_NOT_FOUND) {

			//Print Error if neccessary
			printf("ERROR: Handle was not attached. Reason: %s not available.\n", portName);

		} else {
			printf("ERROR!!!");
		}
	} else {
		//If connected we try to set the comm parameters
		DCB dcbSerialParams = {0};

		//Try to get the current
		if (!GetCommState(this->handle, &dcbSerialParams)) {
			//If impossible, show an error
			printf("failed to get current serial parameters!");
		} else {
			//Define serial connection parameters for the arduino board
			dcbSerialParams.BaudRate=baud;
			dcbSerialParams.ByteSize=8;
			dcbSerialParams.StopBits=ONESTOPBIT;
			dcbSerialParams.Parity=NOPARITY;

			//Set the parameters and check for their proper application
			if(!SetCommState(handle, &dcbSerialParams)) {
				printf("ALERT: Could not set Serial Port parameters");
			} else {
				//If everything went fine we're connected
				this->connected = true;
				//We wait 2s as the arduino board will be reseting
				Sleep(ARDUINO_WAIT_TIME);
			}
		}
	}

}

Serial::~Serial() {
	//Check if we are connected before trying to disconnect
	if(this->connected) {
		//We're no longer connected
		this->connected = false;
		//Close the serial handler
		CloseHandle(this->handle);
	}
}

int Serial::read(char *buffer, unsigned int nbChar) {
	//Number of bytes we'll have read
	DWORD bytesRead;
	//Number of bytes we'll really ask to read
	unsigned int toRead;

	//Use the ClearCommError function to get status info on the Serial port
	ClearCommError(this->handle, &this->errors, &this->status);

	//Check if there is something to read
	if(this->status.cbInQue>0) {
		//If there is we check if there is enough data to read the required number
		//of characters, if not we'll read only the available characters to prevent
		//locking of the application.
		if(this->status.cbInQue>nbChar) {
			toRead = nbChar;
		} else {
			toRead = this->status.cbInQue;
		}

		//Try to read the require number of chars, and return the number of read bytes on success
		if(ReadFile(this->handle, buffer, toRead, &bytesRead, NULL) && bytesRead != 0) {
			return bytesRead;
		}
	}

	//If nothing has been read, or that an error was detected return -1
	return -1;
}

int Serial::write(char *buffer, unsigned int nbChar) {
	DWORD bytesSent;

	//Try to write the buffer on the Serial port
	if(!WriteFile(this->handle, (void *)buffer, nbChar, &bytesSent, 0)) {
		//In case it don't work get comm error and return false
		ClearCommError(this->handle, &this->errors, &this->status);
	}

	return bytesSent;
}

#endif // WIN32
