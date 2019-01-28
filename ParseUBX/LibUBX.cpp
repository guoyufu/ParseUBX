//**************************************************************
// UBX protocol parcing tools
//   - this file implements functions for parsing UBX messages.
//**************************************************************
// Programmer: Ron Denton [RDD]
// Date: 2012 Nov. 13
//**************************************************************
// Change Log:
//   - 2012 Nov. 13 - Added change log. [RDD]
//
//**************************************************************

// included libraries
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "LibUBX.h"
#include <bitset>

using namespace std;

// defined constants


// constructors
UBXMessage::UBXMessage()  // default constructor
{
	header.sync1 = 0xb5;
	header.sysc2 = 0x62;
	header.MessageClass = 0;
	header.MessageID    = 0;
	header.length       = 0;

	payload = 0;  // null pointer to payload
}

UBXMessage::UBXMessage(const UBXMessage& message)  // copy constructor
{
	// use overloaded assignment operator to copy
	*this = message;
}

UBXMessage::UBXMessage(char * buffer, int bufferSize)
{

	UBXHeader * tempHeader;
	tempHeader = reinterpret_cast<UBXHeader*>(buffer);

	header = *tempHeader;  // copy header information from buffer

	// copy payload from buffer
	payload = new U1[header.length];
	memcpy(payload, &buffer[sizeof(header)], header.length);

	/*unsigned char * new_buffer = (unsigned char * )buffer;
	for(int i=0;i < 56;i++)
	{
		printf("%d %d %d\n",i,buffer[6+i],payload[i]);
	}
	printf("206 %d\n",new_buffer[206]);
	printf("207 %d\n",new_buffer[207]);*/

	checksum.ck_A = buffer[sizeof(header) + header.length];
	checksum.ck_B = buffer[sizeof(header) + header.length + 1];
}

// destructor
UBXMessage::~UBXMessage(void)
{
	if(payload != 0)
	{
		delete [] payload;
		payload = 0;
	}
}

// operators
// assignment operator
UBXMessage& UBXMessage::operator=(const UBXMessage &message)
{
	// verify arguments are not the same
	if(this == &message)
		return(*this);

	if(payload != 0)
	{	// target message has a payload
		delete [] payload;  // deallocate old payload array
		payload = 0;
	}

	if(message.payload != 0)
	{	// copied message has a payload array
		payload = new U1[message.header.length];  // allocate payload array for copy
		memcpy(payload, message.payload, message.header.length);
	}

	// copy header and checksum structs
	header   = message.header;
	checksum = message.checksum;

	return(*this);
}

// function implementatons
bool UBXMessage::verifyChecksum(void)
{
	// calculate packet checksum
	U1 ck_A = 0;
	U1 ck_B = 0;

	// add required header fields to checksum
	ck_A = header.MessageClass;
	ck_B = ck_A;
	ck_A += header.MessageID;
	ck_B += ck_A;
	ck_A += (header.length & 0XFF);  // add low order bits of short
	ck_B += ck_A;
	ck_A += (header.length >> 8);     // add high order bits of short
	ck_B += ck_A;

	// add payload bytes to checksum
	for(int i = 0; i < header.length; i++)
	{
		ck_A += payload[i];
		ck_B += ck_A;
	}

	// compare stored vs. calculated checksums
	if(checksum.ck_A != ck_A || checksum.ck_B != ck_B)
		return(false);  // checksums do not match

	// checksums verified correct
	return(true);
}

int UBXMessage::writeCSV(ofstream &outFile)
{
	int bytesWritten = 0;

	// check that file is ready
	if(!outFile.is_open())
		return(0);

	switch(header.MessageClass)
	{
		
		case NAV:   // Navigation results message
			switch(header.MessageID)
			{
				case CLOCK:
					bytesWritten = writeNAV_CLOCK(outFile);
					break;
				case DGPS:
					bytesWritten = writeNAV_DGPS(outFile);
					break;
				case DOP:
					//bytesWritten = writeNAV_DOP(outFile);
					break;
				case POSECEF:
					bytesWritten = writeNAV_POSECEF(outFile);
					break;
				case POSLLH:
					//bytesWritten = writeNAV_POSLLH(outFile);
					break;
				case SBAS:
					bytesWritten = writeNAV_SBAS(outFile);
					break;
				case SOL:
					bytesWritten = writeNAV_SOL(outFile);
					break;
				case STATUS:
					bytesWritten = writeNAV_STATUS(outFile);
					break;
				case SVINFO:
					bytesWritten = writeNAV_SVINFO(outFile);
					break;
				case TIMEGPS:
					bytesWritten = writeNAV_TIMEGPS(outFile);
					break;
				case TIMEUTC:
					bytesWritten = writeNAV_TIMEUTC(outFile);
					break;
				default:
					cout << "Unsupported NAV message ID!" << endl;
			}
			break;
			
		case RXM:  // Reciever manager messages
			switch(header.MessageID)
			{
				case RAW:
					if (header.length <= 500)
					{
						bytesWritten = writeRXM_RAW(outFile);
					}
					break;
				case RAWX:
					if (header.length <= 500)
					{
						bytesWritten = writeRXM_RAWX(outFile);
					}
					break;
				case EPH:
					if (header.length == 104 )
					{
						bytesWritten = writeRXM_EPH(outFile);
					}
					break;
				case SFRBX:
					if (header.length <= 500)
					{
						bytesWritten = writeRXM_SFRBX(outFile);
					}
					break;
				case MEASX:
					if (header.length <= 500)
					{
						bytesWritten = writeRXM_MEASX(outFile);
					}
					break;
				case SFRB:
					//bytesWritten = writeRXM_SFRB(outFile);
					break;
				default:
					cout << "Unsupported RXM message ID!" << endl;
			}
			break;
			
		case AID:  // AssistNow Aiding Messages
			switch(header.MessageID)
			{
				case EPH:
					bytesWritten = writeAID_EPH(outFile);
					break;
				case HUI:
					bytesWritten = writeAID_HUI(outFile);
				default:
					cout << "Unsupported AID message ID!" << endl;
			}
			break;
			
		default:
			//cout << "Unsupported message class!" << endl;
			/*DEBUG-Start*/
			cout << "Unsupported message class!";
			cout << "  Message: 0x";
			cout << hex << setfill('0') << setw(2);
			cout << static_cast<unsigned>(header.MessageClass) << " 0x";
			cout << setw(2) << static_cast<unsigned>(header.MessageID);
			cout << dec << endl;
			/*DEBUG-End*/
	}

	return(bytesWritten);
}

int UBXMessage::writeNAV_CLOCK(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_CLOCK * p_data;
	p_data = reinterpret_cast<UBXPayload_NAV_CLOCK*>(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "CLOCK,";

	// write payload content to output line
	outputLine << p_data->iTOW       << ",";  // GPS Millisecond Time of week (milliseconds)
	outputLine << p_data->clockBias  << ",";  // Clock bias (nanoseconds)
	outputLine << p_data->clockDrift << ",";  // Clock drift (nanoseconds/second)
	outputLine << p_data->timeAcc    << ",";  // Time accuracy estimate
	outputLine << p_data->freqAcc;            // Frequency accuracy estimate
	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeNAV_DGPS(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_DGPS * p_data;
	p_data = reinterpret_cast<UBXPayload_NAV_DGPS*>(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "DGPS,";

	// write payload content to output line
	outputLine << p_data->iTOW                         << ",";  // GPS Millisecond Time of week (milliseconds)
	outputLine << p_data->age                          << ",";  // Age of newest correction data (milliseconds)
	outputLine << p_data->baseID                       << ",";  // DGPS Base Station ID
	outputLine << p_data->baseHealth                   << ",";  // DGPS Base Station Health Status
	outputLine << static_cast<unsigned>(p_data->numCh) << ",";  // number of channels for which correction data follows (# of repeated blocks)
	outputLine << static_cast<unsigned>(p_data->status);        // DGPS Correction Type Status (00 => none, 01 => PR+PRR)

	// get pointer to repeated block
	UBXPayload_NAV_DGPS_rb * p_block;
	p_block = reinterpret_cast<UBXPayload_NAV_DGPS_rb*>(&payload[sizeof(UBXPayload_NAV_DGPS)]);

	// write repeated block data to output line
	for(unsigned int i = 0; i < p_data->numCh; i++)
	{
		// write one blocks data
		outputLine << ",";
		outputLine << static_cast<unsigned>(p_block->svid) << ",";  // Spave Vehicle ID

		outputLine << "0x" << hex << setfill('0') << setw(2);       // bitmask / channel number
		outputLine << static_cast<unsigned>(p_block->flags);        // channel number: 0x01-0x08 => channel on this SV,
		outputLine << dec << ",";                                   // bit flag: 0x10 => DGPS used for this channel

		outputLine << p_block->ageC << ",";  // Age of latest correction data (milliseconds)

		outputLine << setprecision(8) << p_block->prc << ",";  // Pseudo Range Correction (meters)
		outputLine << setprecision(8) << p_block->prrc;        // Pseudo Range Rate Correction (meters/second)

		p_block++;  // advance to next block
	}

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeNAV_DOP(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_DOP * p_data;
	p_data = reinterpret_cast<UBXPayload_NAV_DOP*>(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "DOP,";

	// write payload content to output line
	outputLine << p_data->iTOW << ",";  // GPS Millisecond Time of week (milliseconds)
	outputLine << p_data->gDOP << ",";  // Geometric Dilution of precision (DOP) [scaling x0.01]
	outputLine << p_data->pDOP << ",";  // Position DOP [scaling x0.01]
	outputLine << p_data->tDOP << ",";  // Time DOP [scaling x0.01]
	outputLine << p_data->vDOP << ",";  // Vertical DOP [scaling x0.01]
	outputLine << p_data->hDOP << ",";  // Horizontal DOP [scaling x0.01]
	outputLine << p_data->nDOP << ",";  // Northing DOP [scaling x0.01]
	outputLine << p_data->eDOP;         // Easting DOP [scaling x0.01]

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeNAV_POSECEF(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_POSECEF * p_data;
	p_data = reinterpret_cast<UBXPayload_NAV_POSECEF*>(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "POSECEF,";

	// write payload content to output line
	outputLine << p_data->iTOW  << ",";  // GPS Millisecond Time of week (milliseconds)
	outputLine << p_data->ecefX << ",";  // ECEF X coordinate (centimeters)
	outputLine << p_data->ecefY << ",";  // ECEF Y coordinate (centimeters)
	outputLine << p_data->ecefZ << ",";  // ECEF Z coordinate (centimeters)
	outputLine << p_data->pAcc;          // Position accuracy estimate (centimeters)

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeNAV_POSLLH(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_POSLLH * p_data;
	p_data = reinterpret_cast<UBXPayload_NAV_POSLLH*>(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "POSLLH,";

	// write payload content to output line
	outputLine << p_data->iTOW   << ",";  // GPS Millisecond Time of week (milliseconds)
	outputLine << p_data->lon    << ",";  // Longitude (degrees) [scaling x1e-7]
	outputLine << p_data->lat    << ",";  // Latitude (degrees) [scaling x1e-7]
	outputLine << p_data->height << ",";  // height above ellipsoid (millimeters)
	outputLine << p_data->hMSL   << ",";  // height above mean sea level (millimeters)
	outputLine << p_data->hAcc   << ",";  // horizontal accuracy estimate (millimeters)
	outputLine << p_data->vAcc;           // vertical accuracy estimate (millimeters)

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG*/ cout << outputLine.str();

	return(bytesWritten);
}

int UBXMessage::writeNAV_SBAS(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_SBAS * p_data;
	p_data = reinterpret_cast<UBXPayload_NAV_SBAS*>(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "SBAS,";

	// write payload content to output line
	outputLine << p_data->iTOW << ",";  // GPS Millisecond Time of week (milliseconds)

	outputLine << static_cast<unsigned>(p_data->geo)  << ",";  // PRN Number of the GEO where correction and integrity were acquired
	outputLine << static_cast<unsigned>(p_data->mode) << ",";  // SBAS Mode (0 => disabled, 1 => Enable Integrity, 3 => Enable Testmode)
	outputLine << static_cast<int>(p_data->sys)       << ",";  // SBAS System (-1 => unknown, 0 => WAAS, 1 => EGNOS, 2 => MSAS, 16 => GPS)

	outputLine << "0x" << hex << setfill('0') << setw(2);  // SBAS services available:
	outputLine << static_cast<unsigned>(p_data->service);  // (Bit: 0 => Ranging,   1 => Corrections,
	outputLine << dec << ",";                              //       2 => Integrity, 3 => Testmode)

	outputLine << static_cast<unsigned>(p_data->cnt);  // Number of SV data following (# of repeated blocks)

	// get pointer to repeated block
	UBXPayload_NAV_SBAS_rb * p_block;
	p_block = reinterpret_cast<UBXPayload_NAV_SBAS_rb*>(&payload[sizeof(UBXPayload_NAV_SBAS)]);

	// write repeated block data to output line
	for(unsigned int i = 0; i < p_data->cnt; i++)
	{
		// write one blocks data
		outputLine << ",";
		outputLine << static_cast<unsigned>(p_block->svid) << ",";  // Spave Vehicle ID

		outputLine << "0x" << hex << setfill('0') << setw(2);
		outputLine << static_cast<unsigned>(p_block->flags);         // flags for this SV
		outputLine << dec << ",";

		outputLine << static_cast<unsigned>(p_block->udre)  << ",";  // Monitoring status
		outputLine << static_cast<unsigned>(p_block->svSys) << ",";  // System (0 => WAAS,  1 => EGNOS, 
																	 //         2 => MSAS, 16 => GPS)

		outputLine << "0x" << hex << setfill('0') << setw(2);       // Sevices available:
		outputLine << static_cast<unsigned>(p_block->svService);    // Bit: 0 => Ranging,   1 => Corrections,
		outputLine << dec << ",";                                   //      2 => Integrity, 3 => Testmode

		outputLine << p_block->prc << ",";    // Pseudo Range Correction (centimeters)
		outputLine << p_block->ic;            // Ionosphere Correction (centimeters)

		p_block++;  // advance to next block
	}

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeNAV_SOL(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_SOL * p_data;
	p_data = reinterpret_cast<UBXPayload_NAV_SOL*>(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "SOL,";

	// write payload content to output line
	outputLine << p_data->iTOW   << ",";  // GPS Millisecond Time of week (milliseconds)
	outputLine << p_data->fTOW   << ",";  // Fractional Nanoseconds of rounded iTOW above (nanoseconds) [range: -500000 - 500000]
	outputLine << p_data->week   << ",";  // GPS Week

	outputLine << "0x" << hex << setfill('0') << setw(2);  // Fix type: 
	outputLine << static_cast<unsigned>(p_data->gpsFix);   // 0x00 => No Fix, 0x01 => Dead Reckoning only,  0x02 => 2D-Fix,
	outputLine << dec << ",";                              // 0x03 => 3D Fix, 0x04 => GPS + dead reckoning, 0x05 => Time only fix

	outputLine << "0x" << hex << setfill('0') << setw(2);  // Fix status flags:
	outputLine << static_cast<unsigned>(p_data->flags);    // Bit: 0 => GPSfixOK,      1 => DGPS was used, 
	outputLine << dec << ",";                              //      2 => week is valid, 3 => time of week valid

	outputLine << p_data->ecefX  << ",";  // ECEF X coordinate (centimeters)
	outputLine << p_data->ecefY  << ",";  // ECEF Y coordinate (centimeters)
	outputLine << p_data->ecefZ  << ",";  // ECEF Z coordinate (centimeters)
	outputLine << p_data->pAcc   << ",";  // Position accuracy estimate (centimeters)
	outputLine << p_data->ecefVX << ",";  // ECEF X velocity (centimeters/second)
	outputLine << p_data->ecefVY << ",";  // ECEF Y velocity (centimeters/second)
	outputLine << p_data->ecefVZ << ",";  // ECEF Z velocity (centimeters/second)
	outputLine << p_data->sAcc   << ",";  // Speed accuracy estimate (centimeters/second)
	outputLine << p_data->pDOP   << ",";  // Position DOP [scaling x0.01]

	outputLine << static_cast<unsigned>(p_data->numSV);  // number of SV used in nav solution

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeNAV_STATUS(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_STATUS * p_data;
	p_data = reinterpret_cast<UBXPayload_NAV_STATUS*>(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "STATUS,";

	// write payload content to output line
	outputLine << p_data->iTOW << ",";  // GPS Millisecond Time of week (milliseconds)

	outputLine << "0x" << hex << setfill('0') << setw(2);  // Fix type: 
	outputLine << static_cast<unsigned>(p_data->gpsFix);   //  0x00 => No Fix  
	outputLine << dec << ",";                              //  0x02 => 2D-Fix
														   //  0x01 => Dead Reckoning only
														   //  0x03 => 3D Fix
														   //  0x04 => GPS + dead reckoning
														   //  0x05 => Time only fix

	outputLine << "0x" << hex << setfill('0') << setw(2);  // Nav status flags:
	outputLine << static_cast<unsigned>(p_data->flags);    //  Bit: 0 => GPSfixOK
	outputLine << dec << ",";                              //       1 => DGPS was used
														   //       2 => week is valid
														   //       3 => time of week valid

	outputLine << "0x" << hex << setfill('0') << setw(2);  // fix status flags:
	outputLine << static_cast<unsigned>(p_data->fixStat);  //  Bit: 0   => DGPS Input status 0 none, 1 PR+PRR correction,
	outputLine << dec << ",";                              //       6&7 => map matching status

	outputLine << "0x" << hex << setfill('0') << setw(2);  // Nav output flags 
	outputLine << static_cast<unsigned>(p_data->flags2);   //  Bit: 0&1 => power safe mode state
	outputLine << dec << ",";

	outputLine << p_data->ttff   << ",";  // time to first fix (millisecond time tag)
	outputLine << p_data->msss;           // time since startup/reset (milliseconds)

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeNAV_SVINFO(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_SVINFO * p_data;
	p_data = reinterpret_cast<UBXPayload_NAV_SVINFO*>(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "SVINFO,";

	// write payload content to output line
	outputLine << p_data->iTOW                          << ",";  // GPS Millisecond Time of week (milliseconds)
	outputLine << static_cast<unsigned>(p_data->numCh)  << ",";  // number of channels (# of repeated blocks)
	outputLine << "0x" << hex << setfill('0') << setw(2);           
	outputLine << static_cast<unsigned>(p_data->globalFlags);    // Bits 0..2 => Chip hardware generation
	outputLine << dec;

	// get pointer to repeated block
	UBXPayload_NAV_SVINFO_rb * p_block;
	p_block = reinterpret_cast<UBXPayload_NAV_SVINFO_rb*>(&payload[sizeof(UBXPayload_NAV_SVINFO)]);

	// write repeated block data to output line
	for(unsigned int i = 0; i < p_data->numCh; i++)
	{
		// write one blocks data
		outputLine << ",";
		outputLine << static_cast<unsigned>(p_block->chn)  << ",";  // Channel number
		outputLine << static_cast<unsigned>(p_block->svid) << ",";  // Spave Vehicle ID

		outputLine << "0x" << hex << setw(2);  // Bit 0 => SV used for navigation
		outputLine << static_cast<unsigned>(p_block->flags);   // Bit 1 => Differential correction available for SV
		outputLine << dec << ",";                              // Bit 2 => Orbit information available (almanac or ephemeris)
									                           // Bit 3 => Orbit information is ephemeris
									                           // Bit 4 => SV is unhealthy (should not be used)
															   // Bit 5 => Orbit information is Almanac Plus
															   // Bit 6 => Orbit information is AssistNow Autonomous
															   // Bit 7 => Carrier smoothed psuedorange used

		outputLine << "0x" << hex << setw(2);   // Bits 0-3 => Signal quality indicator: 
		outputLine << static_cast<unsigned>(p_block->quality);  // 0 => channel idle,   1 => channel is searching
		outputLine << dec << ",";                               // 2 => Signal aquired, 3 => signal detected, but unused
	                                                            // 4 => Code lock on signal
	                                                            // 5,6,7 => Code and Carrier locked

		outputLine << static_cast<unsigned>(p_block->cno)  << ",";  // Carrier to Noise Ratio (dbHz)
		outputLine << static_cast<int>(p_block->elev)      << ",";  // Elevation (degrees)
		outputLine << static_cast<int>(p_block->azim)      << ",";  // Azimuth (degrees)
		outputLine << p_block->prRes;                               // Pseudo range residual (centimeters)

		p_block++;  // advance to next block
	}

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeNAV_TIMEGPS(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_NAV_TIMEGPS * p_data = new UBXPayload_NAV_TIMEGPS(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "TIMEGPS,";

	// write payload content to output line
	outputLine << p_data->iTOW   << ",";  // GPS Millisecond Time of week (milliseconds)
	outputLine << p_data->fTOW   << ",";  // Fractional Nanoseconds of rounded iTOW above (nanoseconds) [range: -500000 - 500000]
	outputLine << p_data->week   << ",";  // GPS Week

	outputLine << static_cast<int>(p_data->leapS) << ",";  // leap seconds (GPS->UTC)

	outputLine <<  hex << setfill('0') << setw(2);  // Validity flags:
	outputLine << static_cast<unsigned>(p_data->valid);    //  Bit: 0 => TOW valid, 1 => Week valid,
	outputLine << dec << ",";                              //       2 => leap seconds valid

	outputLine << p_data->tAcc;           // time accuracy estimate (nanoseconds)

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	delete(p_data);

	return(bytesWritten);
}

int UBXMessage::writeNAV_TIMEUTC(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

							  // overlay a struct onto the payload data
	UBXPayload_NAV_TIMEUTC * p_data = new UBXPayload_NAV_TIMEUTC(payload);

	// write message class and Id to output line
	outputLine << "NAV,";
	outputLine << "TIMEUTC,";

	// write payload content to output line
	outputLine << p_data->iTOW << ",";  // GPS Millisecond Time of week (milliseconds)
	outputLine << p_data->tAcc << ",";  // time accuracy estimate (nanoseconds)
	outputLine << p_data->nano << ",";  // Fraction of second (ns)
	outputLine << p_data->year << ",";  // Year
	outputLine << static_cast<unsigned>(p_data->month) << ","; // Month
	outputLine << static_cast<unsigned>(p_data->day) << ",";   // Day of month
	outputLine << static_cast<unsigned>(p_data->hour) << ",";  // Hour of day
	outputLine << static_cast<unsigned>(p_data->min) << ",";   // Minute of hour
	outputLine << static_cast<unsigned>(p_data->sec) << ",";   // Seconds of minute

	outputLine << hex << setfill('0') << setw(2);  // Validity flags:
	outputLine << static_cast<unsigned>(p_data->valid);    //  Bit: 0 => TOW valid, 1 => Week valid,
	
	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	delete(p_data);

	return(bytesWritten);
}

int UBXMessage::writeRXM_RAW(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_RXM_RAW * p_data ;//= new UBXPayload_RXM_RAW(payload);
	p_data = reinterpret_cast<UBXPayload_RXM_RAW*>(payload);

	// get pointer to repeated block
	UBXPayload_RXM_RAW_rb * p_block ;//= new UBXPayload_RXM_RAW_rb(&payload[sizeof(UBXPayload_RXM_RAW)]);
	p_block = reinterpret_cast<UBXPayload_RXM_RAW_rb*>(&payload[8]);

	/*if (p_data->numSV > 15 || p_block->mesQI < 4)
	{
		return 0;
	}*/

	// write message class and Id to output line
	outputLine << "RXM,";
	outputLine << "RAW,";

	// write payload content to output line
	outputLine << p_data->iTOW   << ",";  // Measured GPS Millisecond Time of week, Reciever time (milliseconds)
	outputLine << p_data->week   << ",";  // Measured GPS Week, Reciever time (weeks)
	outputLine << static_cast<unsigned>(p_data->numSV);  // number of SVs (# of repeated blocks)

	// write repeated block data to output line
	for(unsigned int i = 0; i < p_data->numSV; i++)
	{
		// write one blocks data
		outputLine << ",";
		//outputLine << setprecision(15) << p_block->cpMes << ",";  // Carrier phase measurement (L1 cycles)
		outputLine << setprecision(15) << p_block->prMes << ",";  // Pseudorange measurement (meters)
		//outputLine << setprecision(8)  << p_block->doMes << ",";  // Doppler measurement (Hz)

		outputLine << static_cast<unsigned>(p_block->sv)<<','  ;  // space vehicle number
		//outputLine << static_cast<int>(p_block->mesQI)    << ",";  // Nav measurement Quality indicator:
																   //  >=4: PR+DO OK, >=5: PR+DO+CP OK,
																   //  <6: likely loss of carrier lock on previous interval
		outputLine << static_cast<int>(p_block->cno);  // Signal strength C/No. (dbHz)
		//outputLine << static_cast<unsigned>(p_block->lli);         // Loss of lock indicator

		
		p_block++;  // advance to next block
	}

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	//delete(p_data);
	//delete(p_block);

	return(bytesWritten);
}

int UBXMessage::writeRXM_RAWX(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

							  // overlay a struct onto the payload data
	UBXPayload_RXM_RAWX * p_data;//= new UBXPayload_RXM_RAW(payload);
	p_data = reinterpret_cast<UBXPayload_RXM_RAWX*>(payload);

	// get pointer to repeated block
	UBXPayload_RXM_RAWX_rb * p_block;//= new UBXPayload_RXM_RAW_rb(&payload[sizeof(UBXPayload_RXM_RAW)]);
	p_block = reinterpret_cast<UBXPayload_RXM_RAWX_rb*>(&payload[16]);

	/*if (p_data->numSV > 15 || p_block->mesQI < 4)
	{
	return 0;
	}*/

	// write message class and Id to output line
	outputLine << "RXM,";
	outputLine << "RAWX,";

	// write payload content to output line
	outputLine << p_data->rcvTOW << ",";  // Measured GPS Millisecond Time of week, Reciever time (milliseconds)
	outputLine << p_data->week << ",";  // Measured GPS Week, Reciever time (weeks)
	outputLine << static_cast<unsigned>(p_data->numMeas);  // number of SVs (# of repeated blocks)

														 // write repeated block data to output line
	for (unsigned int i = 0; i < p_data->numMeas; i++)
	{
		// write one blocks data
		outputLine << ",";
		//outputLine << setprecision(15) << p_block->cpMes << ",";  // Carrier phase measurement (L1 cycles)
		outputLine << setprecision(15) << p_block->prMes << ",";  // Pseudorange measurement (meters)
																  //outputLine << setprecision(8)  << p_block->doMes << ",";  // Doppler measurement (Hz)

		outputLine << static_cast<unsigned>(p_block->svId) << ',';  // space vehicle number
																  //outputLine << static_cast<int>(p_block->mesQI)    << ",";  // Nav measurement Quality indicator:
																  //  >=4: PR+DO OK, >=5: PR+DO+CP OK,
																  //  <6: likely loss of carrier lock on previous interval
		outputLine << static_cast<int>(p_block->cno);  // Signal strength C/No. (dbHz)
													   //outputLine << static_cast<unsigned>(p_block->lli);         // Loss of lock indicator


		p_block++;  // advance to next block
	}

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	//delete(p_data);
	//delete(p_block);

	return(bytesWritten);
}

int UBXMessage::writeRXM_EPH(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_RXM_EPH * p_data = new UBXPayload_RXM_EPH(payload);
	//p_data = reinterpret_cast<UBXPayload_RXM_EPH*>(payload);

	// write message class and Id to output line
	outputLine << "RXM,";
	outputLine << "EPH";

	// message has 3 forms (2 polling requests, 1 input/output message)
	if(header.length == 1)
	{	// polling packet for 1 SV's ephemeris
		outputLine << ",";
		outputLine << static_cast<unsigned>(payload[0]);  // output SV ID from payload field
		
	}
	else if(header.length > 1 )
	{	// message is an input/output messsage
		// write payload content to output line
		outputLine << ",";
		outputLine << p_data->svid << ",";  // SV ID for this ephemeris data

		outputLine << "0x" << hex << setfill('0');
		outputLine << setw(6) << p_data->how;  // Hand-over Word of first subframe (0 if no data available)
		outputLine << dec;

		if(p_data->how != 0)
		{	// ephemeris data is present in payload
			// get pointer to optional block
			UBXPayload_RXM_EPH_opt * p_block = new UBXPayload_RXM_EPH_opt(payload+8);
			//p_block = reinterpret_cast<UBXPayload_RXM_EPH_opt*>(&payload[sizeof(UBXPayload_RXM_EPH)]);

			outputLine << ",SF1,";
			for(int i = 0; i < 8; i++)
				outputLine  << bitset<30>(p_block->sf1d[i]);
			outputLine << ",SF2,";
			for(int i = 0; i < 8; i++)
				outputLine  <<bitset<30>(p_block->sf2d[i]);
			outputLine << ",SF3,";
			for(int i = 0; i < 8; i++)
				//outputLine << setw(6) << setfill('0') << hex << p_block->sf3d[i]		
				outputLine << bitset<30>(p_block->sf3d[i]);
			delete(p_block);
		}
	}
	// else: message is a poll all SV request (header.length == 0)

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/
	
	delete(p_data);
	

	return(bytesWritten);
}

int UBXMessage::writeRXM_SFRB(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_RXM_SFRB * p_data;
	p_data = reinterpret_cast<UBXPayload_RXM_SFRB*>(payload);

	// write message class and Id to output line
	outputLine << "RXM,";
	outputLine << "SFRB,";

	// write payload content to output line
	outputLine << static_cast<unsigned int>(p_data->chn)  << ",";  // u-Blox channel number
	outputLine << static_cast<unsigned int>(p_data->svid) << ",";  // Space Vehicle Identifier (PRN)

	// output raw subframe buffer data
	for(int i = 0; i < 10; i++)
		outputLine << " " << setw(8) << setfill('0') << hex << p_data->dwrd[i];

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeRXM_SFRBX(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

							  // overlay a struct onto the payload data
	UBXPayload_RXM_SFRBX * p_data;
	p_data = reinterpret_cast<UBXPayload_RXM_SFRBX*>(payload);

	// write message class and Id to output line
	outputLine << "RXM,";
	outputLine << "SFRBX,";

	// write payload content to output line
	outputLine << static_cast<unsigned int>(p_data->chn) << ",";  // u-Blox channel number
	outputLine << static_cast<unsigned int>(p_data->svId) << ",";  // Space Vehicle Identifier (PRN)

																   // output raw subframe buffer data
	for (int i = 0; i < 10; i++)
		outputLine << " " << setw(8) << setfill('0') << hex << p_data->dwrd[i];

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	return(bytesWritten);
}

int UBXMessage::writeRXM_MEASX(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

							  // overlay a struct onto the payload data
	UBXPayload_RXM_MEASX * p_data;
	p_data = reinterpret_cast<UBXPayload_RXM_MEASX*>(payload);

	// get pointer to repeated block
	UBXPayload_RXM_MEASX_rb * p_block;
	p_block = reinterpret_cast<UBXPayload_RXM_MEASX_rb*>(&payload[16]);

	/*if (p_data->numSV > 15 || p_block->mesQI < 4)
	{
	return 0;
	}*/

	// write message class and Id to output line
	outputLine << "RXM,";
	outputLine << "MEASX,";

	// write payload content to output line
	outputLine << p_data->gpsTOW << ","; //GPS measurement reference time (ms)
	//outputLine << p_data->gloTOW << ",";  // GLONASS measurement reference time (ms)
	outputLine << static_cast<unsigned>(p_data->numSV);  // number of SVs (# of repeated blocks)

														   // write repeated block data to output line
	for (unsigned int i = 0; i < p_data->numSV; i++)
	{
		// write one blocks data
		outputLine << ",";
		//outputLine << setprecision(15) << p_block->cpMes << ",";  // Carrier phase measurement (L1 cycles)
		outputLine << setprecision(15) << p_block->mpathIndic << ",";  // multipath index
		outputLine << setprecision(8)  << p_block->dopplerHz << ",";  // Doppler measurement (Hz)

		outputLine << static_cast<unsigned>(p_block->svId) << ',';  // space vehicle number
																	//outputLine << static_cast<int>(p_block->mesQI)    << ",";  // Nav measurement Quality indicator:
																	//  >=4: PR+DO OK, >=5: PR+DO+CP OK,
																	//  <6: likely loss of carrier lock on previous interval
		outputLine << static_cast<int>(p_block->cNo);  // Signal strength C/No. (dbHz)
													   //outputLine << static_cast<unsigned>(p_block->lli);         // Loss of lock indicator


		p_block++;  // advance to next block
	}

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/

	//delete(p_data);
	//delete(p_block);

	return(bytesWritten);
}


int UBXMessage::writeAID_EPH(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_AID_EPH * p_data ;
	p_data = reinterpret_cast<UBXPayload_AID_EPH*>(payload);

	// write message class and Id to output line
	outputLine << "AID,";
	outputLine << "EPH";

	// message has 3 forms (2 polling requests, 1 input/output message)
	if(header.length == 1)
	{	// polling packet for 1 SV's ephemeris
		outputLine << ",";
		outputLine << static_cast<unsigned>(payload[0]);  // output SV ID from payload field
	}
	else if(header.length > 1)
	{	// message is an input/output messsage
		// write payload content to output line
		outputLine << ",";
		outputLine << p_data->svid << ",";  // SV ID for this ephemeris data

		outputLine << "0x" << hex << setfill('0');
		outputLine << setw(6) << p_data->how;  // Hand-over Word of first subframe (0 if no data available)
		outputLine << dec;

		if(p_data->how != 0)
		{	// ephemeris data is present in payload
			// get pointer to optional block
			UBXPayload_AID_EPH_opt * p_block;
			p_block = reinterpret_cast<UBXPayload_AID_EPH_opt*>(&payload[sizeof(UBXPayload_AID_EPH)]);

			outputLine << ",SF1,";
			for(int i = 0; i < 8; i++)
				outputLine << " " << setw(6) << setfill('0') << hex << p_block->sf1d[i];
			outputLine << ",SF2,";
			for(int i = 0; i < 8; i++)
				outputLine << " " << setw(6) << setfill('0') << hex << p_block->sf2d[i];
			outputLine << ",SF3,";
			for(int i = 0; i < 8; i++)
				outputLine << " " << setw(6) << setfill('0') << hex << p_block->sf3d[i];
		}
	}
	// else: message is a poll all SV request (header.length == 0)

	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/
	return(bytesWritten);
}

int UBXMessage::writeAID_HUI(ofstream &outFile)
{
	int bytesWritten = 0;     // number of bytes written to output file
	stringstream outputLine;  // string stream in which to build output line for file

	// overlay a struct onto the payload data
	UBXPayload_AID_HUI * p_data = new UBXPayload_AID_HUI(payload);
			
	
	// check if klob bit is valid
	if (header.length == 0 )
	{
		return 0;
	}
	
	// write message class and Id to output line
	outputLine << "AID,";
	outputLine << "HUI,";
	
	//outputLine << hex << p_data->health <<',' ;
	outputLine << p_data->utcTOW << ',';
	outputLine << p_data->utcWNT << ',';
	outputLine << dec << setprecision(16) << p_data->klobA0 <<',' ; 
	outputLine << dec << setprecision(16) << p_data->klobA1  <<',' ;
	outputLine << dec << setprecision(16) << p_data->klobA2  <<',' ;
	outputLine << dec << setprecision(16) << p_data->klobA3  <<',' ;
	outputLine << dec << setprecision(16) << p_data->klobB0  <<',' ; 
	outputLine << dec << setprecision(16) << p_data->klobB1 <<',' ;
	outputLine << dec << setprecision(16) << p_data->klobB2  <<',' ;
	outputLine << dec << setprecision(16) << p_data->klobB3  << ',';
	outputLine << p_data->flags;
	outputLine << endl;

	// get number of bytes to be written
	bytesWritten = outputLine.str().length();

	// write line to output file
	outFile << outputLine.str();
	/*DEBUG cout << outputLine.str();*/
	delete(p_data);
	return(bytesWritten);
}