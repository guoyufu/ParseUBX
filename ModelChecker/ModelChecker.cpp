#include <iostream>
#include <fstream>
#include "../ParseUBX/LibUBX.h"
#include "ModelChecker.h"
using namespace std;

ModelChecker::ModelChecker(string name, AlertCollection * pac)
{
	int res;
	fname = name;
	up = UBXParser();
	res = up.open(fname);
	// TODO, how to check whether the file exist?
	input_source = UBX_FILE;
	ac = pac;
}


ModelChecker::~ModelChecker()
{
	if( input_source == UBX_FILE)
	{
		fname = "";
	}
}

int ModelChecker::read_next()
{
	int res;
	UBXMessage um;
	res = up.read_next_ubx(um);
	if(res != 0)
	{
		cout << "error read next ubx message" << endl;
		return 1;
	}
	// TODO, it might be that the ubxparser package provider 
	// do not have any packages
	
	// It should be the UP that send message 
	// ModelChecker have a receiver that receive the message
	// There might be other receiver too.

	// check which list should the current um should be in
	switch(um.header.MessageClass)
	{
	case RXM:
		switch(um.header.MessageID)
		{
		case SVSI:
			rxmsvsi_list.push_back(um);
			// TODO, 
			// update the health information
			break;
		case RAW:
			rxmraw_list.push_back(um);
			// TODO, 
			// using the est function here to
			// get the estimated value 
			// check whether the raw and Est is good
			check_message();
			break;
		}
		break;
	case NAV:
		switch(um.header.MessageID)
		{
		case SOL:
			navsol_list.push_back(um);
			break;
		}
		break;
	case AID:
		switch(um.header.MessageID)
		{
		case HUI:
			aidhui_list.push_back(um);
			break;
		case EPH:
			aideph_list.push_back(um);
			break;
		}
		break;
	}
	return 0;
}

int ModelChecker::check_message()
{

	// Step 1, get all satellite
	// remove unhealhy satellite
	// Healthy information from satellite
	

	// Step 2, get the prMes raw pseudorange
	// from RXM RAW package

	
	// Step 3, 
	// TODO, add resolve function for the AID package
	// GET AID-EPH
	// GET AID_HUI
	// GET NAV_SOL


	return 0;
}