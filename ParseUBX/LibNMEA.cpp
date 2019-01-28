//**************************************************************
// NMEA protocol parcing tools
//   - this file implements functions for parsing NMEA messages.
//**************************************************************
// Programmer: Ron Denton [RDD]
// Date: 2012 Nov. 16
//**************************************************************
// Change Log:
//   - 2012 Nov. 16 - Added change log. [RDD]
//
//**************************************************************

// included libraries
#include <sstream>
#include <iostream>
#include <iomanip>
#include "LibNMEA.h"

using namespace std;

// defined constants

// verifyChecksum: verifies the check sum of a sting
//   Note: this function assumes the presence of a leading '$' and trailing '*'
bool verifyChecksum(string &message)
{
	// The checksum is the 8-bit exclusive OR of all characters in the packet, 
	//   including the "," delimiters, between -- but not including -- the "$" 
	//   and "*" delimiters.

	size_t first, last, index;  // indices into a string
	unsigned char  chksum;       // calculated check sum
	unsigned short msgChksum;    // message check sum

	first = message.find_first_of('$') + 1;         // start at char after '$'
	last  = message.find_first_of('*', first) - 1;  // stop at char before '*'

	// preform xor of all characters from first to last
	chksum = message.at(first);
	for(index = first + 1; index <= last; index++)
	{
		chksum ^= message.at(index);
	}

	// convert message checksum into unsigned char
	stringstream ss(message.substr(last+2,2));   // string stream used for type conversion
	ss >> hex >> msgChksum;
	if(msgChksum != static_cast<unsigned short>(chksum))
		return(false);  // checksums do not match

	// checksums verified correct
	return(true);
}


