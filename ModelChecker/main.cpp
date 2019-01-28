// included libraries
#include <iostream>
#include <fstream>
#include <string>
#include "../ParseUBX/LibUBX.h"
#include "../ParseUBX/LibNMEA.h"
#include "../ParseUBX/ParseUBX.h"
#include "ModelChecker.h"
using namespace std;

// main program module
int main(int argc, char* argv[])
{
	AlertCollection	ac = AlertCollection();
	string fname = "../ParseUBX/t.ubx";
	ModelChecker mc = ModelChecker(fname,&ac);
	while(true)
	{
		mc.read_next();
		break;
	}

	return(0);
}
