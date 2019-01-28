//**********************************************************************
// File:			GPSUtilities.h
// Programmer:		Ron Denton
// Description:		Misc. Methods for GPS data processing
// Created:         07/20/2012
// Last modified:	07/20/2012
//**********************************************************************
//
// 
//
//**********************************************************************
// Change Log:
//
//**********************************************************************
#pragma once

// defined constants
#define SPEED_OF_LIGHT      299792458   // in m/s
#define MAX_ITERATIONS      20          // maximum number of iteration
#define SOLUTION_TOLERANCE  0.10        // GPS solution tolerance
#define DEBUG               false       // turn off debug features

// system specific file location
#define ALMANAC_PATH "C:\\GPSData\\Almanacs\\"
#define ALMANAC_FILE_EXTENSION ".AL3"
#define FINAL_EPHEMERIS_PATH "C:\\GPSData\\igscb\\"
#define FINAL_EPHEMERIS_NAME "igs"
#define FINAL_EPHEMERIS_FILE_EXTENSION ".sp3"
#define RAPID_EPHEMERIS_PATH "C:\\GPSData\\igscb\\"
#define RAPID_EPHEMERIS_NAME "igr"
#define RAPID_EPHEMERIS_FILE_EXTENSION ".sp3"
#define PATH_SEPARATOR "\\"

// included libraries
#include "SP3EphemerisStore.hpp"
#include "SEMAlmanacStore.hpp"
#include "DayTime.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"

using namespace gpstk;


// method prototypes
void loadEphemeris(SP3EphemerisStore &ephem, unsigned int gpsWeek);
void loadRapidEphemeris(SP3EphemerisStore &ephem, unsigned int gpsWeek, unsigned int dayOfWeek);
void loadAlmanac(SEMAlmanacStore &almanac, DayTime &startTime, unsigned int days);

int calculateDOP(Matrix<double> &matrixDOP, Matrix<double> &dataSV, Vector<double> solution);
int solutionNLLS(Matrix<double> &dataSV, Vector<double> &solution);

