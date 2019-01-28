#ifndef PARSE_UBX_H
#define PARSE_UBX_H

#include "LibUBX.h"
#include "libNMEA.h"

// defined constants
#define BUFFER_SIZE 4096

class UBXParser
{
public:
	UBXParser():log(0),in_file_p(NULL){};
	int open(string fname);			// initialize the name of the ubx file
	
	// TODO, define the message here
	int read_next_ubx(UBXMessage &um);

	int writecsv(string outname);	// write out the package in csv format
private:
	int log;
	//ifstream in_file;
	//basic_ifstream<unsigned char> in_file;
	// The copy of in_file is not permitted.
	basic_ifstream<unsigned char> * in_file_p;
	unsigned char buffer[BUFFER_SIZE];
	// forward declarations
	int readNMEA(unsigned char* buffer, int bufferSize);
	int readUBX(unsigned char* buffer, int bufferSize);
	int processNMEAMessage(ofstream &outFile, unsigned char* buffer, int bufferSize);
	int processUBXMessage(ofstream &outFile, unsigned char* buffer, int bufferSize);
};

#endif