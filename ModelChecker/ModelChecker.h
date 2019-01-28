#ifndef MODEL_CHECKER_H
#define MODEL_CHECHER_H

#include <vector>
#include "../ParseUBX/ParseUBX.h"

class GPSAlert
{
public:
	GPSAlert(){};
	GPSAlert(string alert){};
private:
	string message;
};

class AlertCollection
{
public:
	AlertCollection(){};
private:
	vector<GPSAlert> alert_list;
};

#define UBX_FILE 0

class ModelChecker
{
public:
	ModelChecker(){};
	ModelChecker(string fname, AlertCollection *ac);

	~ModelChecker();
	int read_next();
	int check_message();


private:
	int input_source;
	string fname;
	UBXParser up;

	AlertCollection * ac;	

	// Message List
	vector<UBXMessage> rxmsvsi_list;
	vector<UBXMessage> rxmraw_list;
	vector<UBXMessage> navsol_list;
	vector<UBXMessage> aidhui_list;
	vector<UBXMessage> aideph_list;

	//
};

#endif