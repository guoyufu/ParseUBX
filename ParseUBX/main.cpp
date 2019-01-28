// included libraries
#include <iostream>
#include <fstream>
#include <string>
#include "LibUBX.h"
#include "libNMEA.h"
#include "ParseUBX.h"

// main program module
int main(int argc, char* argv[])
{
	UBXParser up;
	int res;

	//get the input data
	string input;
	//cout<<"Enter input file (.ubx):\n";
	//getline(cin,input);
	input = "ds3_r2.ubx";
	res = up.open(input);
	if(res != 0)
	{
		return res;
	}

	string output;
	//cout<<"Enter output file (.csv):\n";
	//getline(cin,output);
	output = "ds3-r2.csv";
	res = up.writecsv(output);
	if(res != 0)
	{
		return res;
	}

	return(0);
}
