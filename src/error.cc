#include <iostream>
#include "error.h"

using namespace std;

/* The error handling class */

/* Name storage and retrieval routines */

error::error(scanner* scanner_mod)  /* the constructor */
{
	//Populate errorlist with reserved words
	errorlist.push_back("Error 0x0000: Must have device list first in definition file, initialised with 'DEVICES'"); //0
	errorlist.push_back("Error 0x0001: Must have connection list second (after devices and before monitors) in definition file, initialised by 'CONNECTIONS'"); //1
	errorlist.push_back("Error 0x0002: Must have monitor list last (after devices list and connections) in definition file, initialised by 'MONITORS'"); //2
	errorlist.push_back("Error 0x0003: Must have at least one device in devices list"); //3
	errorlist.push_back("Error 0x0004: Need device type for device definition"); //4
	errorlist.push_back("Error 0x0005: Expecting device name or END after semicolon (device name must start with letter)"); //5 device list
	errorlist.push_back("Error 0x0006: Clock must have integer clock number greater than 0"); //6
	errorlist.push_back("Error 0x0007: Switch must be set to either 0 or 1"); //7
	errorlist.push_back("Error 0x0008: Must specify an integer number of inputs between 1 and 16 to a GATE"); //8
	errorlist.push_back("Error 0x0006: Clock must have integer clock number greater than 0"); //9
	errorlist.push_back("Error 0x000A: Need colon after name for CLOCK/SWITCH/GATE type"); //10
	errorlist.push_back("Error 0x000B: Name must start with letter and only contain letter number and '_'"); //11
	errorlist.push_back("Error 0x000C: Connection must start with the name of a device"); //12
	errorlist.push_back("Error 0x000D: Expecting device name or END after semicolon (device name must start with letter)"); //13 connection list
	errorlist.push_back("Error 0x000E: Expect a dot after DTYPE"); //14
	errorlist.push_back("Error 0x000F: Input device called in connection list does not exist"); //15
	errorlist.push_back("Error 0x0010: Must specify output to connect to input with equals sign "); //16
	errorlist.push_back("Error 0x0011: Must specify valid input gate after dot"); //17
	errorlist.push_back("Error 0x0012: Need to specify valid iput gate seperated from device name by a '.'"); //18
	errorlist.push_back("Error 0x0013: Output device called in connection list does not exist"); //19
	errorlist.push_back("Error 0x0014: Monitor must start with the name of a valid device"); //20
	errorlist.push_back("Error 0x0015: Expecting device name or END after semicolon (device name must start with letter)"); //21 monitor list
	errorlist.push_back("Error 0x0016: Expect a dot after DTYPE as must specify output to monitor in monitor list"); //22
	errorlist.push_back("Error 0x0017: Bad device monitor"); //23
	errorlist.push_back("Error 0x0018: Need semicolon at end of previous line"); //24
	errorlist.push_back("Error 0x0019: Must only be one devices list"); //25
	errorlist.push_back("Error 0x001A: There must be one 'DEVICES' block, it may not have been initialised properly");//26
	errorlist.push_back("Error 0x001B: Devices block must be initialised with 'DEVICES'"); //27
	errorlist.push_back("Error 0x001C: Must only be one connections list"); //28
	errorlist.push_back("Error 0x001D: Must only be one monitors list"); //29
	errorlist.push_back("Error 0x001E: There must be one 'CONNECTIONS' block, it may not have been initialised properly");//30
	errorlist.push_back("Error 0x001F: There must be one 'MONITOR' block, it may not have been initialised properly");//31
	errorlist.push_back("Error 0x0020: Block must be terminated with 'END'");//32
	
	errorCount = 0;
	warningCount = 0;
	
	warninglist.push_back("Warning 0x0000: You have not specfied any conenctions. Please check this is what is required");//0
	warninglist.push_back("Warning 0x0001: You have not specfied any monitors. Please check this is what is required");//1
	
	smz = scanner_mod;
}

void error::newError(int errorCode){
	if (errorCode>=0 && errorCode < errorlist.size())
	{
		smz->writelineerror();
		cout << errorlist[errorCode] << endl;
		errorCount ++;
	}
	else{
		cout << "Internal software error: Error code " << errorCode << " does not exist" << endl;
	}
}

void error::newWarning(int warningCode){
	cout << warninglist[warningCode] << endl; //don't display where warning occurs
	warningCount ++;
}

bool error::anyErrors(){
	if (errorCount==0){
		if (warningCount==1){
			cout << "There are no errors and 1 warning" << endl;
		}
		if (warningCount>1){
			cout << "There are no errors and " << warningCount << " warnings" << endl;
		}
		return 0;
	}
	if (errorCount==1){
		if (warningCount==0){
			cout << "There is 1 error" << endl;
		}
		else if (warningCount==1){
			cout << "There is 1 error and 1 warning" << endl;
		}
		else if (warningCount>1){
			cout << "There is 1 error and " << warningCount << "warnings" << endl;
		}
		return 1;
	}
	if (errorCount>1){
				if (warningCount==0){
			cout << "There are " << errorCount << " errors" << endl;
		}
		else if (warningCount==1){
			cout << "There are " << errorCount << " errors and 1 warning" << endl;
		}
		else if (warningCount>1){
			cout << "There are " << errorCount << " errors and " << warningCount << "warnings" << endl;
		}
		return 1;
	}
}
