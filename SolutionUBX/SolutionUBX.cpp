
// defined constants

// included libraries
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

// included GPSTk libraries
#include "SP3EphemerisStore.hpp"
#include "DayTime.hpp"
#include "Xvt.hpp"
#include "SatID.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"
#include "Bancroft.hpp"

// other included libraries
#include "..\GPSUtilities\GPSUtilities.h"

using namespace std;
using namespace gpstk;

// custom data types
struct RXM_RAW_SV_DATA {
	double cpMes;  // Carrier phase measurement (L1 cycles)
	double prMes;  // Pseudorange measurement (meters)
	double doMes;  // Doppler measurement (Hz)
	int    sv;     // space vehicle number
	int    mesQI;  // Nav measurement Quality indicator (>=4: PR+DO OK, >=5: PR+DO+CP OK,
	               //                                    <6: likely loss of carrier lock on previous interval
	int    cno;    // Signal strength C/No. (dbHz)
	int    lli;    // Loss of lock indicator
};
struct RXM_RAW_DATA {
	int iTOW;   // Measured GPS Millisecond Time of week, Reciever time (milliseconds)
	int week;   // Measured GPS Week, Reciever time (weeks)
	int numSV;  // number of SVs (# of repeated blocks)
	vector< RXM_RAW_SV_DATA > svData;
};


// forward declarations
bool parseLine(string &fileLine, RXM_RAW_DATA &lineData);

// main program module
int main(int argc, char* argv[])
{
	int messagesProcessed = 0;
	int errorcode;
	ifstream inFile;
	ofstream outFile;
	string fileLine;
	RXM_RAW_DATA lineData;

	try
	{
		//Bancroft solver;                // create a Bancroft GPS solver
		Matrix<double> dataSV(0,4);       // solver input data
		Matrix<double> matrixDOP(0,4);    // dilution of precision matrix
		Vector<double> solution(4);       // GPS solution
		Vector<double> dataRow(4);        // one data row for solver data matrix
		SP3EphemerisStore ephem;     // class to store ephemeris data
		DayTime time;                // store time of interest
		double userClockBias;        // approximate receiver clock bias

		SatID sv(1,SatID::systemGPS);  // GPS satellite identifier (initialized to PRN #1)

		// set times
		short int week;
		cout<<"Please enter the GPS week: (1715)\n";
		cin>>week;
		int day;
		cout<<"Please enter the GPS day of week: (3)\n";
		cin>>day;
		double startTime;  // GPS time: 11/21/2012 20:47:33
		cout<<"Please enter the start time: (334053.0)\n";
		cin>>startTime;
		
		cin.clear();cin.ignore(INT_MAX,'\n');

		// load rapid ephemeris for GPS week 1715 day 3 (Wednesday)
		loadRapidEphemeris(ephem, static_cast<unsigned int>(week), day);

		//get the input data
		string input;
		//"C:\\Users\\Denton.R\\Desktop\\Multipath\\Site1-Northside-21Nov2012.csv"
		cout<<"Enter input file (.csv):\n";
		getline(cin,input);

		// open log file
		inFile.open(input.c_str());
		if(!inFile.is_open())
		{
			cout << "Unable to open input file!" << endl << endl;
			return(-1);
		}

		//get the output data
		string output;
		//"C:\\Temp\\Site1-Solutions-21Nov2012.csv"
		cout<<"Enter output file (.csv):\n";
		getline(cin,output);

		// open output file
		outFile.open(output.c_str());
		if(!outFile.is_open())
		{
			cout << "Unable to open output file!" << endl << endl;
			inFile.close();
			return(-1);
		}

		// write header to file
		outFile << "TOW(s),X(m),Y(m),Z(m),b(s),SVs,GDOP" << endl;

		cout << "Processing Logfile Messages" << endl;
		cout << "Message Count:" << endl;

		// initialize solution to all zeros (center of earth in ECEF)
		for(unsigned int i = 0; i < solution.size(); i++)
			solution(i) = 0.0;
		userClockBias = 0.0;  // initial clock bias unknown

		// process messages from file
		while(!inFile.eof())
		{
			// read line from file
			getline(inFile, fileLine);

			// parse data from line
			if(parseLine(fileLine, lineData))
			{	// extracted valid RXM-RAW data message
				time.setGPS(week, lineData.iTOW/1000.0);   // load time of interest
				dataSV.resize(0,4);  // clear input data matrix

				// get SV position from IGS Ephemeris
				for(int i = 0; i < lineData.numSV; i++)
				{
					sv.id = lineData.svData.at(i).sv;
					if(sv.isValid() && lineData.svData.at(i).sv != 27)
					{
						Xvt xvt = ephem.getXvt(sv,time);  // get rapid ephemeris SV position

						// load SV data into solution input matrix
						dataRow[0] = xvt.x[0];
						dataRow[1] = xvt.x[1];
						dataRow[2] = xvt.x[2];

						// load pseudorange into data row
						dataRow[3] = lineData.svData.at(i).prMes;

						// modify pseudorange for known errors
						dataRow[3] += xvt.dtime * SPEED_OF_LIGHT;  // SV clock bias
						dataRow[3] -= userClockBias;               // appoximate user clock bias
						//dataRow[3] -= relativisticError;  // relativistic correction for time
						//dataRow[3] -= atmosphericError;   // atmospheric delays

						// push_back data row onto solver input data matrix
						dataSV = dataSV && dataRow;
					}
				}

				// calculate GPS solution from data (previous solution used as initial guess)
				errorcode = solutionNLLS(dataSV, solution);
				//errorcode = solver.Compute(dataSV, solution);
				if(errorcode == 0)
				{
					// compute clock bias for this solution
					userClockBias += solution[3];  // residual clock bias in solution is amount 
												   //  clock bias varied from previous solution

					// write solution data to output file
					outFile << lineData.iTOW / 1000.0          << ",";
					outFile << setprecision(15) << solution[0] << ",";
					outFile << setprecision(15) << solution[1] << ",";
					outFile << setprecision(15) << solution[2] << ",";
					//outFile << setprecision(15) << solution[3]/SPEED_OF_LIGHT;
					outFile << setprecision(15) << userClockBias/SPEED_OF_LIGHT;

					// write number of SVs used to output file
					outFile << "," << dataSV.rows();

					// get DOP for this solution
					errorcode = calculateDOP(matrixDOP, dataSV, solution);
					if(errorcode >= 0)
						outFile << "," << sqrt(trace(matrixDOP));  // print results

					outFile << endl;
				}

				cout << "\r" << ++messagesProcessed;
			}
		}
	}
	catch(Exception error)
	{
		cout << error << endl;
		exit(-1);
	}

	return(0);
}


bool parseLine(string &fileLine, RXM_RAW_DATA &lineData)
{
	size_t first, last;      // indices into a string
	stringstream ss;         // string stream used to convert strings to values
	RXM_RAW_SV_DATA tempSV;  // temporary storage of SV data parsed from file line

	if(fileLine.substr(0,8) != "RXM,RAW,")
		return(false);  // not correct string type

	// clear linedata SV data to prepare for new data
	lineData.svData.clear();

	// delete class and id from line
	fileLine = fileLine.erase(0,8);

	// get values from line
	first = 0;
	last  = fileLine.find_first_of(',', first);

	ss << fileLine.substr(first, last - first);
	ss >> lineData.iTOW;
	ss.clear();         // clear string stream to get ready for next use
	ss.str(string());

	first = last + 1;
	last  = fileLine.find_first_of(',', first);

	ss << fileLine.substr(first, last - first);
	ss >> lineData.week;
	ss.clear();         // clear string stream to get ready for next use
	ss.str(string());

	first = last + 1;
	last  = fileLine.find_first_of(',', first);

	ss << fileLine.substr(first, last - first);
	ss >> lineData.numSV;
	ss.clear();         // clear string stream to get ready for next use
	ss.str(string());

	for(int i = 0; i < lineData.numSV; i++)
	{
		first = last + 1;
		last  = fileLine.find_first_of(',', first);

		ss << fileLine.substr(first, last - first);
		ss >> tempSV.cpMes;
		ss.clear();         // clear string stream to get ready for next use
		ss.str(string());

		first = last + 1;
		last  = fileLine.find_first_of(',', first);

		ss << fileLine.substr(first, last - first);
		ss >> tempSV.prMes;
		ss.clear();         // clear string stream to get ready for next use
		ss.str(string());

		first = last + 1;
		last  = fileLine.find_first_of(',', first);

		ss << fileLine.substr(first, last - first);
		ss >> tempSV.doMes;
		ss.clear();         // clear string stream to get ready for next use
		ss.str(string());

		first = last + 1;
		last  = fileLine.find_first_of(',', first);

		ss << fileLine.substr(first, last - first);
		ss >> tempSV.sv;
		ss.clear();         // clear string stream to get ready for next use
		ss.str(string());

		first = last + 1;
		last  = fileLine.find_first_of(',', first);

		ss << fileLine.substr(first, last - first);
		ss >> tempSV.mesQI;
		ss.clear();         // clear string stream to get ready for next use
		ss.str(string());

		first = last + 1;
		last  = fileLine.find_first_of(',', first);

		ss << fileLine.substr(first, last - first);
		ss >> tempSV.cno;
		ss.clear();         // clear string stream to get ready for next use
		ss.str(string());

		first = last + 1;
		last  = fileLine.find_first_of(',', first);

		ss << fileLine.substr(first, last - first);
		ss >> tempSV.lli;
		ss.clear();         // clear string stream to get ready for next use
		ss.str(string());

		lineData.svData.push_back(tempSV);
	}

	return(true);
}




