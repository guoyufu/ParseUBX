//**************************************************************
// NMEA protocol parcing tools
//   - this library provides data stuctures and methods for
//     decodeing NMEA log files.
//**************************************************************
// Programmer: Ron Denton [RDD]
// Date: 2012 Nov. 16
//**************************************************************
// Change Log:
//   - 2012 Nov. 16 - Added change log. [RDD]
//
//**************************************************************
#ifndef LIBNMEA_H
#define LIBNMEA_H

// defined constants

// included libraries
#include <string>

using namespace std;

// custom data types

// function prototypes
bool verifyChecksum(string &message);

#endif // LIBNMEA_H
