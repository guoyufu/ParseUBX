#include <iostream>
#include <fstream>

#include "ParseUBX.h"
using namespace std;

int UBXParser::open(string fname)
{
	// TODO, close the in_file if it is open
	if( in_file_p != NULL && in_file_p->is_open() )
	{
		in_file_p->close();
		in_file_p = NULL;
	}

	if( in_file_p == NULL)
	{
		in_file_p = new basic_ifstream<unsigned char>();
	}

	// Step 1 : check whether the file exist
	// TODO
	in_file_p->open(fname.c_str(), ios::in|ios::binary);	
	if(!in_file_p->is_open())
	{
		cout << "Unable to open input file!" << endl << endl;
		in_file_p->close();
		in_file_p = NULL;
		return 1;
	}

	// Step 2 : check the magic number of the file to 
	// test whether it is UBX file
	// TODO

	// Step 3 : open the file open handler
	return 0;
}

int UBXParser::writecsv(string outname)
{
	int res = 0;
	// Step 1 : 
	ofstream out_file;
	
	int messageLength;
	int messagesProcessed = 0;

	out_file.open(outname.c_str(),ios::out);
	if(!out_file.is_open())
	{
		cout << "Unable to open output file!" << endl << endl;
		in_file_p->close();
		return 1;
	}

	if(log)
	{
		cout << "Processing Logfile Messages" << endl;
		cout << "Message Count:" << endl;
	}

	// process messages from file
	while(!in_file_p->eof())
	{
		// find start of a message
		in_file_p->read(&buffer[0], 1);
		
		if(buffer[0] == '$')
		{

			/*DEBUG cout << "Found NMEA message..." << endl;*/
			messageLength = readNMEA(buffer, BUFFER_SIZE);
			res = processNMEAMessage(out_file, buffer, messageLength);
			messagesProcessed++;
		}
		else if(static_cast<unsigned char>(buffer[0]) == 0xb5)
		{
			if(!in_file_p->eof())
			{
				in_file_p->read(&buffer[1], 1);
				if(buffer[1] = 'b')
				{
					/*DEBUG cout << "Found UBX message..." << endl;*/
					messageLength = readUBX(buffer, BUFFER_SIZE);
					res = processUBXMessage(out_file, buffer, messageLength);
					messagesProcessed++;
				}
			}
		}

		if( res != 0 )
		{
			out_file.close();
			break;
		}
		cout << "\r" << messagesProcessed << " ";
		if (messagesProcessed == 12760) {
			int dfasdfa;
			dfasdfa = 1;
		}
	}

	cout << endl;
	cout << "All messages processed." << endl;

	return 0;
}

int UBXParser::read_next_ubx(UBXMessage & um)
{
	int messageLength = 0;

	while(true){
		if( in_file_p->eof() )
		{
			cout << "end of file, cannot read more" << endl;
			return 1;
		}
		
		// find start of a message
		in_file_p->read(&buffer[0], 1);		
		if(buffer[0] == '$')
		{
			messageLength = readNMEA(buffer, BUFFER_SIZE);
			continue;
		}
		else if(static_cast<unsigned char>(buffer[0]) == 0xb5)
		{
			if(!in_file_p->eof())
			{
				in_file_p->read(&buffer[1], 1);
				//if(buffer[1] == 'b')
				if(buffer[1] == 'b')
				{
					messageLength = readUBX(buffer, BUFFER_SIZE);
					um = UBXMessage(reinterpret_cast<char *>(buffer), messageLength);
					//UBXMessage message(reinterpret_cast<char *>(buffer), bufferSize);
					if(!um.verifyChecksum())
					{
						cout << "check sum error"<< endl;
						return 1;
					}
					else
					{
						return 0;
					}
				}
			}
			else
			{
				cout << "read file error" << endl;
				return 1;
			}
		}
	}

	return 1;
}

int UBXParser::readNMEA(unsigned char* buffer, int bufferSize)
{
	int index = 0;
	// read NMEA message from file into buffer
	while(!in_file_p->eof() && buffer[index] != '\n')
	{	// '$' is already in buffer at buffer[0], remaining chars start at buffer[1]
		index++;
		in_file_p->read(&buffer[index], 1);
	}
	// return length of message (index is position of last char)
	return (index+1);
}

int UBXParser::readUBX(unsigned char* buffer, int bufferSize)
{
	UBXHeader * header;
	// read UBX header to get length to read
	in_file_p->read(&buffer[2], sizeof(UBXHeader) - 2);
	header = reinterpret_cast<UBXHeader*>(buffer);
	// read bytes (length + 2 bytes for checksum)
	in_file_p->read(&buffer[sizeof(UBXHeader)], header->length + sizeof(UBXChecksum));
	// compute and return overall message length
	return(sizeof(UBXHeader) + header->length + sizeof(UBXChecksum));
}

int UBXParser::processNMEAMessage(ofstream &outFile,unsigned char* buffer, int bufferSize)
{
	string message(reinterpret_cast<char *>(buffer), bufferSize);  // convert buffered message to a string
	if (message.size() < 5) return 0;
																   // write NMEA message to output file
	if(verifyChecksum(message))
	{
		outFile << message.substr(0,message.length() - 2);  // strip off <CR><LF>
		outFile << endl;
		/*DEBUG cout << message;*/
	}
	else
	{
		cout << "Checksum error!" << endl;
		return 1;
	}
	
	return 0;
}

int UBXParser::processUBXMessage(ofstream &outFile,unsigned char* buffer, int bufferSize)
{
	//string message(reinterpret_cast<char *>(buffer), bufferSize);  // convert buffered message to a string
	UBXMessage message(reinterpret_cast<char *>(buffer), bufferSize);
	if(message.verifyChecksum())
	{
		message.writeCSV(outFile);
	}
	else
	{
		cout << "Checksum error!" << endl;
		return 1;
	}
	return 0;
}

