//**********************************************************************
// File:			GPSUtilities.cpp
// Programmer:		Ron Denton
// Description:		Misc. Methods for GPS data processing
// Created:         06/27/2012
// Last modified:	06/28/2012
//**********************************************************************
//
//
//
//
//**********************************************************************
// Change Log:
//
//**********************************************************************

// included libraries
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cmath>

#include "GPSUtilities.h"

using namespace std;
using namespace gpstk;


void loadEphemeris(SP3EphemerisStore &ephem, unsigned int gpsWeek)
{
	string ephemFileName;
	ostringstream week;

	string path(FINAL_EPHEMERIS_PATH);
	string baseFileName(FINAL_EPHEMERIS_NAME);
	string separator(PATH_SEPARATOR);
	string fileExt(FINAL_EPHEMERIS_FILE_EXTENSION);

	cout << "Loading ephemeris data for GPS week " << gpsWeek << "...";

	week << setw(4) << setfill('0') << gpsWeek;

	// load final (precise) ephemeris for GPS week
	ephemFileName = path + week.str() + separator + baseFileName + week.str() + '0' + fileExt;
	ephem.loadFile(ephemFileName.c_str());  // load ephemeris
	ephemFileName = path + week.str() + separator + baseFileName + week.str() + '1' + fileExt;
	ephem.loadFile(ephemFileName.c_str());  // load ephemeris
	ephemFileName = path + week.str() + separator + baseFileName + week.str() + '2' + fileExt;
	ephem.loadFile(ephemFileName.c_str());  // load ephemeris
	ephemFileName = path + week.str() + separator + baseFileName + week.str() + '3' + fileExt;
	ephem.loadFile(ephemFileName.c_str());  // load ephemeris
	ephemFileName = path + week.str() + separator + baseFileName + week.str() + '4' + fileExt;
	ephem.loadFile(ephemFileName.c_str());  // load ephemeris
	ephemFileName = path + week.str() + separator + baseFileName + week.str() + '5' + fileExt;
	ephem.loadFile(ephemFileName.c_str());  // load ephemeris
	ephemFileName = path + week.str() + separator + baseFileName + week.str() + '6' + fileExt;
	ephem.loadFile(ephemFileName.c_str());  // load ephemeris

	cout << "Done!" << endl;
	return;
}

void loadRapidEphemeris(SP3EphemerisStore &ephem, unsigned int gpsWeek, unsigned int dayOfWeek)
{
	string ephemFileName;
	ostringstream week;
	ostringstream day;

	string path(RAPID_EPHEMERIS_PATH);
	string baseFileName(RAPID_EPHEMERIS_NAME);
	string separator(PATH_SEPARATOR);
	string fileExt(RAPID_EPHEMERIS_FILE_EXTENSION);

	cout << "Loading rapid ephemeris data for GPS week " << gpsWeek;
	cout << ", Day " << dayOfWeek << "...";

	week << setw(4) << setfill('0') << gpsWeek;
	day  << setw(1) << dayOfWeek;

	// load final (precise) ephemeris for GPS week
	ephemFileName = path + week.str() + separator + baseFileName + week.str() + day.str() + fileExt;
	ephem.loadFile(ephemFileName.c_str());  // load ephemeris

	cout << "Done!" << endl;
	return;
}


void loadAlmanac(SEMAlmanacStore &almanac, DayTime &startTime, unsigned int days)
{
	unsigned int daysToLoad;
	DayTime loadTime;

	string almanacFileName;
	ostringstream year;
	ostringstream dayOfYear;

	string path(ALMANAC_PATH);
	string separator(PATH_SEPARATOR);
	string fileExt(ALMANAC_FILE_EXTENSION);

	loadTime = startTime;
	loadTime -= 86400;      // back one day (in seconds)
	daysToLoad = days + 2;  // loading one day before and after window of interest

	almanac.timeOfInterest = startTime;  // give almanac a hint on correct epoch

	cout << "Loading almanac data...";

	while(daysToLoad > 0)
	{
		year      << setw(4) << setfill('0') << loadTime.DOYyear();
		dayOfYear << setw(3) << setfill('0') << loadTime.DOYday();

		almanacFileName = path + year.str() + separator + dayOfYear.str() + fileExt;
		almanac.loadFile(almanacFileName.c_str());

		loadTime += 86400;  // advance to next day
		daysToLoad--;
		year.str(string());
		dayOfYear.str(string());
	}

	cout << "Done!" << endl;
	return;
}


int calculateDOP(Matrix<double> &matrixDOP, Matrix<double> &dataSV, Vector<double> solution)
{
	// validate/prepare input
	if(solution.size() != 4)
		return(-1);
	if(dataSV.rows() < 4)
		return(-2);
	if(dataSV.cols() != 4)
		return(-3);
	matrixDOP.resize(0,4);

	Matrix<double> position(0,4);
	Vector<double> tempRow(4);
	double distSV;

	// build position matrix
	for(unsigned int i = 0; i < dataSV.rows(); i++)
	{
		// calculate distance from user to i_th SV
		distSV  = (solution(0) - dataSV(i,0)) * (solution(0) - dataSV(i,0));
		distSV += (solution(1) - dataSV(i,1)) * (solution(1) - dataSV(i,1));
		distSV += (solution(2) - dataSV(i,2)) * (solution(2) - dataSV(i,2));
		distSV = sqrt(distSV);

		// compute a row of the position matrix
		tempRow(0) = (solution(0) - dataSV(i,0)) / distSV;
		tempRow(1) = (solution(1) - dataSV(i,1)) / distSV;
		tempRow(2) = (solution(2) - dataSV(i,2)) / distSV;
		tempRow(3) = 1.0;

		//add row to postion matrix
		position = position && tempRow;
	}

	// compute generalized inverse of position matrix
	try
	{
		matrixDOP = inverseChol(transpose(position)*position);
	}
	catch(...)
	{
		return(-4);
	}

	return(0);
}

int solutionNLLS(Matrix<double> &dataSV, Vector<double> &solution)
{
	// validate input
	if(dataSV.rows() < 4)
		return(-1);
	if(solution.size() != 4)
		return(-2);

	// initialize
	Matrix<double> jacobian(dataSV.rows(),4);
	Matrix<double> invJacobian;
	Vector<double> rangesSV(dataSV.rows());
	Vector<double> pseudoRanges(dataSV.rows());
	Vector<double> shiftVector(4,0.0);
	Vector<double> positionSV(3);
	double positionShiftMag = 1e22;

	// extract pseudorange vector
	for(unsigned int i = 0; i < dataSV.rows(); i++)
		pseudoRanges(i) = dataSV(i,3);


	// iterate
	int itr = 0;
	while(itr < MAX_ITERATIONS && positionShiftMag > SOLUTION_TOLERANCE)
	{
		// build current Jacobian
		for(unsigned int i = 0; i < dataSV.rows(); i++)
		{
			rangesSV(i) = 0.0;
			for(unsigned int j = 0; j < 3; j++)
			{
				positionSV(j) = dataSV(i,j) - solution(j);
				rangesSV(i) += positionSV(j) * positionSV(j);
			}
			rangesSV(i) = sqrt(rangesSV(i));

			for(unsigned int j = 0; j < 3; j++)
			{
				jacobian(i,j) = -1 * positionSV(j) / rangesSV(i);
			}
			jacobian(i,3) = 1.0; //SPEED_OF_LIGHT;
		}

		if(DEBUG)
		{	// dump jacobian
			cout << "\nJacobian:" << endl;
			for(unsigned int i = 0; i < jacobian.rows(); i++)
			{
				for(unsigned int j = 0; j < jacobian.cols(); j++)
					cout << setprecision(15) << jacobian(i,j) << " ";
				cout << endl;
			}
			cout << endl;
		}

		// compute generalized inverse of Jacobian
		try
		{
			invJacobian = inverseChol(transpose(jacobian)*jacobian);
		}
		catch(...)
		{
			if(DEBUG)
			{	// dump jacobian
				cout << "\nError inverting Jacobian:" << endl;
				for(unsigned int i = 0; i < jacobian.rows(); i++)
				{
					for(unsigned int j = 0; j < jacobian.cols(); j++)
						cout << setprecision(15) << jacobian(i,j) << " ";
					cout << endl;
				}
				cout << endl;

				// dump input data
				cout << "\nSpace vehicle data input:" << endl;
				for(unsigned int i = 0; i < dataSV.rows(); i++)
				{
					for(unsigned int j = 0; j < dataSV.cols(); j++)
						cout << setprecision(15) << dataSV(i,j) << " ";
					cout << endl;
				}
				cout << endl;

				// dump initial solution
				cout << "\nInitial solution input:" << endl;
				for(unsigned int i = 0; i < solution.size(); i++)
					cout << setprecision(15) << solution(i) << " ";
				cout << endl;
			}
			return(-3);
		}

		shiftVector = (invJacobian * transpose(jacobian)) * (pseudoRanges - rangesSV);

		if(DEBUG)
		{
			for(unsigned int k = 0; k < shiftVector.size(); k++)
				cout << shiftVector(k) << " ";
			cout << endl;
		}

		// update solution using shift vector
		solution += shiftVector;

		// update termination conditions
		positionShiftMag = sqrt(shiftVector(0) * shiftVector(0) +
								shiftVector(1) * shiftVector(1) +
								shiftVector(2) * shiftVector(2));
		itr++;
	}

	if(itr >= MAX_ITERATIONS)
		return(-4);

	// save final clock bias term in solution
	// Note: the time residual in the shift vector is the clock bias of the solution
	solution(3) = shiftVector(3);

	if(DEBUG)
	{
		for(unsigned int k = 0; k < shiftVector.size(); k++)
			cout << shiftVector(k)/SPEED_OF_LIGHT << " ";
		cout << endl;
	}

	return(0);
}

