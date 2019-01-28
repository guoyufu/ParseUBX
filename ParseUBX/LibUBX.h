//**************************************************************
// UBX protocol parcing tools
//   - this library provides data stuctures and methods for
//     decodeing binary UBX log files.
//**************************************************************
// Programmer: Ron Denton [RDD], Guoyu Fu
// Date: 2012 Nov. 12
//**************************************************************
// Change Log:
//   - 2012 Nov. 12 - Added change log. [RDD]
//   - 2014
//   - 2017
//**************************************************************
#ifndef LIBUBX_H
#define LIBUBX_H

// defined constants
// *** UBX Message Classes ***
#define NAV 0x01   // Navigation results
#define RXM 0x02   // Reciever manager messages
#define AID 0x0B   // AssistNow Aiding Messages
//#define TIM 0x0D   // Timing messages
// Note: other messages classes not supported...

// *** UBX Message IDs ***
// --- NAV Class ---
#define CLOCK   0x22
#define DGPS    0x31
#define DOP     0x04
#define POSECEF 0x01
#define POSLLH  0x02
#define SBAS    0x32
#define SOL     0x06
#define STATUS  0x03
#define SVINFO  0x30
#define TIMEGPS 0x20
#define TIMEUTC 0x21
// --- RXM Class ---
#define EPH     0x31
#define RAW     0x10
#define SFRB    0x11
#define SVSI    0x20
#define SFRBX   0x13
#define RAWX    0x15
#define MEASX   0x14
// --- AID Class ---
#define HUI     0x02
#define EPH     0x31
// Note: other messages IDs not supported...

// included libraries
#include <fstream>

using namespace std;

// custom data types
// *** UBX data types ***
typedef unsigned char  U1; 
typedef signed   char  I1;
typedef unsigned char  X1;
typedef unsigned short U2;
typedef signed   short I2;
typedef unsigned short X2;
typedef unsigned long  U4;
typedef signed   long  I4;
typedef unsigned long  X4;
typedef float  R4;
typedef double R8;
typedef char   CH;

// basic UBX protocol message structures
struct UBXHeader {
	U1 sync1;
	U1 sysc2;
	U1 MessageClass;
	U1 MessageID;
	U2 length;
};

struct UBXChecksum {
	U1 ck_A;
	U1 ck_B;
};

// UBX message payload structures
struct UBXPayload_NAV_CLOCK {
	U4 iTOW;        // GPS Millisecond Time of week (milliseconds)
	I4 clockBias;   // Clock bias (nanoseconds)
	I4 clockDrift;  // Clock drift (nanoseconds/second)
	U4 timeAcc;     // Time accuracy estimate
	U4 freqAcc;     // Frequency accuracy estimate
};

struct UBXPayload_NAV_DGPS {
	U4 iTOW;        // GPS Millisecond Time of week (milliseconds)
	I4 age;         // Age of newest correction data (milliseconds)
	I2 baseID;      // DGPS Base Station ID
	I2 baseHealth;  // DGPS Base Station Health Status
	U1 numCh;       // number of channels for which correction data follows (# of repeated blocks)
	U1 status;      // DGPS Correction Type Status (00 => none, 01 => PR+PRR)
	U2 reserved1;   // reserved space
};
struct UBXPayload_NAV_DGPS_rb {  // repeated block for NAV-DGPS
	U1 svid;   // Spave Vehicle ID
	U1 flags;  // bitmask / channel number: 0x01-0x08 => channel on this SV,
	           //                           0x10      => DGPS used for this channel
	           //                           0x20-0x80 => reserved
	U2 ageC;   // Age of latest correction data (milliseconds)
	R4 prc;    // Pseudo Range Correction (meters)
	R4 prrc;   // Pseudo Range Rate Correction (meters/second)
};

struct UBXPayload_NAV_DOP {
	U4 iTOW;  // GPS Millisecond Time of week (milliseconds)
	U2 gDOP;  // Geometric Dilution of precision (DOP) [scaling x0.01]
	U2 pDOP;  // Position DOP [scaling x0.01]
	U2 tDOP;  // Time DOP [scaling x0.01]
	U2 vDOP;  // Vertical DOP [scaling x0.01]
	U2 hDOP;  // Horizontal DOP [scaling x0.01]
	U2 nDOP;  // Northing DOP [scaling x0.01]
	U2 eDOP;  // Easting DOP [scaling x0.01]
};

struct UBXPayload_NAV_POSECEF {
	U4 iTOW;   // GPS Millisecond Time of week (milliseconds)
	I4 ecefX;  // ECEF X coordinate (centimeters)
	I4 ecefY;  // ECEF Y coordinate (centimeters)
	I4 ecefZ;  // ECEF Z coordinate (centimeters)
	U4 pAcc;   // Position accuracy estimate (centimeters)
};

struct UBXPayload_NAV_POSLLH {
	U4 iTOW;    // GPS Millisecond Time of week (milliseconds)
	I4 lon;     // Longitude (degrees) [scaling x1e-7]
	I4 lat;     // Latitude (degrees) [scaling x1e-7]
	I4 height;  // height above ellipsoid (millimeters)
	I4 hMSL;    // height above mean sea level (millimeters)
	U4 hAcc;    // horizontal accuracy estimate (millimeters)
	U4 vAcc;    // vertical accuracy estimate (millimeters)
};

struct UBXPayload_NAV_SBAS {
	U4 iTOW;         // GPS Millisecond Time of week (milliseconds)
	U1 geo;          // PRN Number of the GEO where correction and integrity were acquired
	U1 mode;         // SBAS Mode (0 => disabled, 1 => Enable Integrity, 3 => Enable Testmode)
	I1 sys;          // SBAS System (-1 => unknown, 0 => WAAS, 1 => EGNOS, 2 => MSAS, 16 => GPS)
	X1 service;      // SBAS services available (Bit: 0 => Ranging,   1 => Corrections, 
	                 //                               2 => Integrity, 3 => Testmode)
	U1 cnt;          // Number of SV data following (# of repeated blocks)
	U1 reserved[3];  // reserved
};
struct UBXPayload_NAV_SBAS_rb {  // repeated block for NAV-SBAS
	U1 svid;       // Spave Vehicle ID
	U1 flags;      // flags for this SV
	U1 udre;       // Monitoring status
	U1 svSys;      // System (-1 => unknown, 0 => WAAS, 1 => EGNOS, 2 => MSAS, 16 => GPS)
	U1 svService;  // Sevices available (same as service in UBXPayload_NAV_SBAS)
	U1 reserved1;  
	I2 prc;        // Pseudo Range Correction (centimeters)
	U2 reserved2;   
	I2 ic;         // Ionosphere Correction (centimeters)
};

struct UBXPayload_NAV_SOL {
	U4 iTOW;    // GPS Millisecond Time of week (milliseconds)
	I4 fTOW;    // Fractional Nanoseconds of rounded iTOW above (nanoseconds) [range: -500000 - 500000]
	I2 week;    // GPS Week
	U1 gpsFix;  // Fix type: 0x00 => No Fix, 0x01 => Dead Reckoning only, 0x02 => 2D-Fix,
	            //           0x03 => 3D Fix, 0x04 => GPS + dead reckoning, 0x05 => Time only fix
	X1 flags;   // Fix status flags  (Bit: 0 => GPSfixOK,      1 => DGPS was used, 
	            //                         2 => week is valid, 3 => time of week valid)
	I4 ecefX;   // ECEF X coordinate (centimeters)
	I4 ecefY;   // ECEF Y coordinate (centimeters)
	I4 ecefZ;   // ECEF Z coordinate (centimeters)
	U4 pAcc;    // Position accuracy estimate (centimeters)
	I4 ecefVX;  // ECEF X velocity (centimeters/second)
	I4 ecefVY;  // ECEF Y velocity (centimeters/second)
	I4 ecefVZ;  // ECEF Z velocity (centimeters/second)
	U4 sAcc;    // Speed accuracy estimate (centimeters/second)
	U2 pDOP;    // Position DOP [scaling x0.01]
	U1 reserved1;
	U1 numSV;   // number of SV used in nav solution
	U4 reserved2;
};

struct UBXPayload_NAV_STATUS {
	U4 iTOW;     // GPS Millisecond Time of week (milliseconds)
	U1 gpsFix;   // Fix type: 0x00 => No Fix, 0x01 => Dead Reckoning only, 0x02 => 2D-Fix,
	             //           0x03 => 3D Fix, 0x04 => GPS + dead reckoning, 0x05 => Time only fix
	X1 flags;    // Nav status flags  (Bit: 0 => GPSfixOK,      1 => DGPS was used, 
	             //                         2 => week is valid, 3 => time of week valid)
	X1 fixStat;  // fix status flags  (Bit: 0 => DGPS Input status 0 none, 1 PR+PRR correction,
	             //                         6&7 => map matching status)
	X1 flags2;   // Nav output flags  (Bit: 0&1 => power safe mode state)
	U4 ttff;     // time to first fix (millisecond time tag)
	U4 msss;     // time since startup/reset (milliseconds)
};

struct UBXPayload_NAV_SVINFO {
	U4 iTOW;         // GPS Millisecond Time of week (milliseconds)
	U1 numCh;        // number of channels (# of repeated blocks)
	X1 globalFlags;  // Chip hardware generation
	U2 reserved;
};
struct UBXPayload_NAV_SVINFO_rb {  // repeated block for NAV-SVINFO
	U1 chn;      // Channel number
	U1 svid;     // Spave Vehicle ID
	X1 flags;    // Bit 0 => SV used for navigation
	             // Bit 1 => Differential correction available for SV
	             // Bit 2 => Orbit information available (almanac or ephemeris)
	             // Bit 3 => Orbit information is ephemeris
	             // Bit 4 => SV is unhealthy (should not be used)
	             // Bit 5 => Orbit information is Almanac Plus
	             // Bit 6 => Orbit information is AssistNow Autonomous
	             // Bit 7 => Carrier smoothed psuedorange used
	X1 quality;  // Signal quality indicator: 0 => channel idle,   1 => channel is searching
	             //                           2 => Signal aquired, 3 => signal detected, but unused
	             //                           4 => Code lock on signal
	             //                           5,6,7 => Code and Carrier locked
	U1 cno;      // Carrier to Noise Ratio (dbHz)
	I1 elev;     // Elevation (degrees)
	I2 azim;     // Azimuth (degrees)
	I4 prRes;    // Pseudo range residual (centimeters)
};

struct UBXPayload_NAV_TIMEGPS {
	U4 iTOW;   // GPS Millisecond Time of week (milliseconds)
	I4 fTOW;   // Fractional Nanoseconds of rounded iTOW above (nanoseconds) [range: -500000 - 500000]
	I2 week;   // GPS Week
	I1 leapS;  // leap seconds (GPS->UTC)
	X1 valid;  // Validity flags (Bit: 0 => TOW valid, 1 => Week valid, 2 => leap seconds valid)
	U4 tAcc;   // time accuracy estimate (nanoseconds)
	UBXPayload_NAV_TIMEGPS(U1* payload){
		this->iTOW = *((U4 *)payload);
		this->fTOW = *((I4 *)(payload+4));
		this->week = *((I2 *)(payload+8));
		this->leapS = *((I1 *)(payload+10));
		this->valid = *((X1 *)(payload+11));
		this->tAcc = *((U4 *)(payload+12));
	}
};

struct UBXPayload_NAV_TIMEUTC {
	U4 iTOW;   // GPS Millisecond Time of week (milliseconds)
	U4 tAcc;   // time accuracy estimate (nanoseconds)
	I4 nano;   // Fraction of second (ns)
	U2 year;   // Year
	U1 month;  // Month
	U1 day;    // Day of month
	U1 hour;   // Hour of day
	U1 min;    // Minute of hour
	U1 sec;    // Seconds of minute
	X1 valid;  // Validity flags (Bit: 0 => TOW valid, 1 => Week valid, 2 => leap seconds valid)
	
	UBXPayload_NAV_TIMEUTC(U1* payload) {
		this->iTOW = *((U4 *)payload);
		this->tAcc = *((U4 *)(payload + 4));
		this->nano = *((I4 *)(payload + 8));
		this->year = *((U2 *)(payload + 12));
		this->month = *((U1 *)(payload + 14));
		this->day = *((U1 *)(payload + 15));
		this->hour = *((U1 *)(payload + 16));
		this->min = *((U1 *)(payload + 17));
		this->sec = *((U1 *)(payload + 18));
	}
};

struct UBXPayload_AID_EPH {
	U4 svid;   // SV ID for this ephemeris data
	U4 how;    // Hand-over Word of first subframe (0 if no data available)
};

struct UBXPayload_AID_HUI {
	X4 health;  // Bitmask, every bit represenst a GPS SV (1-32). If the bit is set the SV is healthy.
	R8 utcA0;  // UTC - parameter A0
	R8 utcA1;  // UTC - parameter A1
	I4 utcTOW;  // UTC - reference time of week
	I2 utcWNT;  // UTC - reference week number
	I2 utcLS;  // UTC - time difference due to leap seconds before event
	I2 utcWNF;  // UTC - week number when next leap second event occurs
	I2 utcDN;  // UTC - day of week when next leap second event occurs
	I2 utcLSF;  // UTC - time difference due to leap seconds after event
	I2 utcSpare;  // UTC - Spare to ensure structure is a multiple of 4 bytes
	R4 klobA0;  // Klobuchar - alpha 0
	R4 klobA1;   // Klobuchar - alpha 1
	R4 klobA2;  // Klobuchar - alpha 2
	R4 klobA3;   // Klobuchar - alpha 3
	R4 klobB0;  // Klobuchar - beta 0
	R4 klobB1;  // Klobuchar - beta 1
	R4 klobB2;  // Klobuchar - beta 2
	R4 klobB3;  // Klobuchar - beta 3
	X4 flags;

	UBXPayload_AID_HUI(U1* payload){
		this->utcTOW = *((I4 *)(payload+20));
		this->utcWNT = *((I2 *)(payload+24));
		this->klobA0 = *((R4 *)(payload+36));
		this->klobA1 = *((R4 *)(payload+40));
		this->klobA2 = *((R4 *)(payload+44));
		this->klobA3 = *((R4 *)(payload+48));
		this->klobB0 = *((R4 *)(payload+52));
		this->klobB1 = *((R4 *)(payload+56));
		this->klobB2 = *((R4 *)(payload+60));
		this->klobB3 = *((R4 *)(payload+64));
		this->flags = *((X4 *)(payload+68));
	}

};

struct UBXPayload_AID_EPH_opt {  // optional data block for AID_EPH
	U4 sf1d[8];  // Subframe 1 Words 3 -> 10
	U4 sf2d[8];  // Subframe 2 Words 3 -> 10
	U4 sf3d[8];  // Subframe 3 Words 3 -> 10
};

struct UBXPayload_RXM_RAW {
	I4 iTOW;   // Measured GPS Millisecond Time of week, Reciever time (milliseconds)
	I2 week;   // Measured GPS Week, Reciever time (weeks)
	U1 numSV;  // number of SVs (# of repeated blocks)
	U1 reserved;
	UBXPayload_RXM_RAW(U1* payload){
		this->iTOW = *((I4 *)(payload));
		this->week = *((I2 *)(payload+4));
		this->numSV = *((U1 *)(payload+6));
	}
};
struct UBXPayload_RXM_RAW_rb {
	R8 cpMes;  // Carrier phase measurement (L1 cycles)
	R8 prMes;  // Pseudorange measurement (meters)
	R4 doMes;  // Doppler measurement (Hz)
	U1 sv;     // space vehicle number
	I1 mesQI;  // Nav measurement Quality indicator (>=4: PR+DO OK, >=5: PR+DO+CP OK,
	           //                                    <6: likely loss of carrier lock on previous interval
	I1 cno;    // Signal strength C/No. (dbHz)
	U1 lli;    // Loss of lock indicator
	UBXPayload_RXM_RAW_rb(U1 * payload){
		this->prMes = *((R8 *)(payload+16));
		this->sv = *((U1 *)(payload+28));
		this->mesQI = *((I1 *)(payload+29));
	}
};


struct UBXPayload_RXM_RAWX {
	R8 rcvTOW;   // Measured GPS Millisecond Time of week, Reciever time (seconds)
	U2 week;   // Measured GPS Week, Reciever time (weeks)
	I1 leapS;  // GPS leap seconds (GPS-UTC)
	U1 numMeas;  // number of SVs (# of repeated blocks)
	X1 recStat;  // Receiver tracking status bitfield
	U1 reserved1, reserved2, reserved3;
	UBXPayload_RXM_RAWX(U1* payload) {
		this->rcvTOW = *((R8 *)(payload));
		this->week = *((U2 *)(payload + 8));
		this->numMeas = *((U1 *)(payload + 11));
	}
};
struct UBXPayload_RXM_RAWX_rb {
	R8 prMes;  // Pseudorange measurement (meters)
	R8 cpMes;  // Carrier phase measurement (L1 cycles)
	R4 doMes;  // Doppler measurement (Hz)
	U1 gnssId; // GNSS identifier
	U1 svId;   // space vehicle number
	U1 reserved2;
	U1 freqId; // Only used for GLONASS: this is the frequency slot + 7 (range from 0 to 13)
	U2 locktime; // Carrier phase locktime counter (ms)
	U1 cno;    // Signal strength C/No. (dbHz)
	X1 prStdev; // Estimated pseudorange measurement standard deviation (m)
	X1 cpStdev; // Estimated carrier phase measurement standard deviation (cycles)
	X1 doStdev; // Estimated Doppler measurement standard deviation. (Hz)
	X1 trkStat; // Tracking status bitfield 
	U1 reserved3;
	UBXPayload_RXM_RAWX_rb(U1 * payload) {
		this->prMes = *((R8 *)(payload + 16));
		this->gnssId = *((U1 *)(payload + 36));
		this->svId = *((U1 *)(payload + 37));
		this->cno = *((U1 *)(payload + 42));
	}
};

struct UBXPayload_RXM_SFRB {
	U1 chn;       // Channel number
	U1 svid;      // SV ID of Satellite transmitting subframe
	X4 dwrd[10];  // Words of data
};

struct UBXPayload_RXM_SFRBX {
	U1 gnssId; // GNSS identifier
	U1 svId; // satellite identifier
	U1 reserved1; 
	U1 freqId;   // Only used for GLONASS
	U1 numWords; // THe number of data words contained in this message 
	U1 chn;    // The tracking channel number the message was received on
	U1 version;  // Message version
	U1 reserved2;
	U4 dwrd[10];
	UBXPayload_RXM_SFRBX(U1 * payload) {
		this->gnssId = *((U1 *)(payload));
		this->svId = *((U1 *)(payload+1));
		this->numWords = *((U1 *)(payload+4));
	}
};

struct UBXPayload_RXM_MEASX {
	U1 version;  // Message version
	U1 reserved11, reserved12, reserved13;
	U4 gpsTOW; // GPS measurement reference time (ms)
	U4 gloTOW; // GLONASS measurement reference time (ms)
	U4 bdsTOW; // BeiDou measurement reference time (ms)
	U1 reserved21, reserved22, reserved23, reserved24;
	U4 qzssTOW; // QZSS measurement reference time (ms)
	U2 gpsTOWacc;  // GPS measurement reference time accuracy (ms)
	U2 gloTOWacc;  // GLONASS measurement reference time accuracy (ms)
	U2 bdsTOWacc;  // BeiDou measurement reference time accuracy (ms)
	U1 reserved31, reserved32;
	U2 qzssTOWacc;  // QZSS measurement reference time accuracy (ms)
	U1 numSV;  // Number of satellites in repeated block
	U1 flags;
	U1 reserved4[8];
	UBXPayload_RXM_MEASX(U1 * payload) {
		this->gpsTOW = *((U4 *)(payload + 4));
		this->gloTOW = *((U4 *)(payload + 8));
		this->bdsTOW = *((U4 *)(payload + 12));
		this->qzssTOW = *((U4 *)(payload + 20));
		this->numSV = *((U1 *)(payload + 34));
	}
};
struct UBXPayload_RXM_MEASX_rb {
	U1 gnssId;
	U1 svId;
	U1 cNo;     // carrier noise ratio
	U1 mpathIndic; // multipath index
	I4 dopplerMS;  // Doppler measurement
	I4 dopplerHz;  // Doppler measurement
	U2 wholeChips; // whole value of the code phase measurement
	U2 fracChips;  // fractional value of the code phase measurement
	U4 codePhase;  // Code phase (ms)
	U1 intCodePhase;  // Integer part of the code phase
	U1 pseuRangeRMSErr; // pseudorange RMS error index
	U1 reserved5[2];
	UBXPayload_RXM_MEASX_rb(U1 * payload) {
		this->gnssId = *((U1 *)(payload + 44));
		this->svId = *((U1 *)(payload + 45));
		this->cNo = *((U1 *)(payload + 46));
		this->mpathIndic = *((U1 *)(payload + 47));
		this->dopplerMS = *((I4 *)(payload + 48));
		this->dopplerHz = *((I4 *)(payload + 52));
		this->pseuRangeRMSErr = *((U1 *)(payload + 65));
	}
};

struct UBXPayload_RXM_EPH {
	U4 svid;   // SV ID for this ephemeris data
	U4 how;    // Hand-over Word of first subframe (0 if no data available)
	UBXPayload_RXM_EPH(U1 *payload){
		this->svid = *((U4 *) payload);
		this->how = *((U4 *) (payload+4));
	}
};
struct UBXPayload_RXM_EPH_opt {  // optional data block for AID_EPH
	U4 sf1d[8];  // Subframe 1 Words 3 -> 10
	U4 sf2d[8];  // Subframe 2 Words 3 -> 10
	U4 sf3d[8];  // Subframe 3 Words 3 -> 10
	UBXPayload_RXM_EPH_opt(U1 *payload){
		//this->sf1d = (U4 *) (payload + 8);
		//this->sf1d = new U4[8];
		memcpy(this->sf1d,payload+8,8*sizeof(U4));
		memcpy(this->sf2d,payload+40,8*sizeof(U4));
		memcpy(this->sf3d,payload+72,8*sizeof(U4));
	
	}


};

// definition of UBXMessage class
class UBXMessage
{
	public:
		UBXHeader   header;
		U1 *        payload;
		UBXChecksum checksum;

		// constructors
		UBXMessage();                           // default constructor
		UBXMessage(const UBXMessage& message);      // copy constructor
		UBXMessage(char * buffer, int bufferSize);

		// destructor
		~UBXMessage(void);

		// operators
		UBXMessage& operator=(const UBXMessage &message);  // assignment

		// methods
		bool verifyChecksum(void);
		int  writeCSV(ofstream &outFile);

	private:
		// methods to output CSV data from UBX messages
		int writeNAV_CLOCK(ofstream &outFile);
		int writeNAV_DGPS(ofstream &outFile);
		int writeNAV_DOP(ofstream &outFile);
		int writeNAV_POSECEF(ofstream &outFile);
		int writeNAV_POSLLH(ofstream &outFile);
		int writeNAV_SBAS(ofstream &outFile);
		int writeNAV_SOL(ofstream &outFile);
		int writeNAV_STATUS(ofstream &outFile);
		int writeNAV_SVINFO(ofstream &outFile);
		int writeNAV_TIMEGPS(ofstream &outFile);
		int writeNAV_TIMEUTC(ofstream &outFile);
		int writeRXM_RAW(ofstream &outFile);
		int writeRXM_RAWX(ofstream &outFile);
		int writeRXM_SFRB(ofstream &outFile);
		int writeRXM_SFRBX(ofstream &outFile);
		int writeRXM_MEASX(ofstream &outFile);
		int writeRXM_EPH(ofstream &outFile);
		int writeAID_EPH(ofstream &outFile);
		int writeAID_HUI(ofstream &outFile);

};

// function prototypes


#endif  // LIBUBX_H
