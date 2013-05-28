#include <iostream>
#include "parser.h"
#include "error.h"

using namespace std;

/* The error handling class */

/* Name storage and retrieval routines */

error::error(void)  /* the constructor */
{
	//Populate errorlist with reserved words
	errorlist.push_back("Error 0: Must have device list first in definition file"); //0
	errorlist.push_back("Error 1: Must have connection list second (after definition file and before mointors) in definition file"); //1
	errorlist.push_back("Error 2: Must have monitor list last in definition file"); //2
	errorlist.push_back("Error 3: Must have at least one device in devices list"); //3
	errorlist.push_back("Error 4: Need device type for device definition"); //4
	errorlist.push_back("Error 5: New device must have a name or must end with END not semicolon"); //5
	errorlist.push_back("Error 6: Clock must have intiger clock number greater than 0"); //6
	errorlist.push_back("Error 7: Switch must be set to either 0 or 1"); //7
	errorlist.push_back("Error 8: Must specify an intiger number of inputs between 1 and 16 to a GATE"); //8
	errorlist.push_back("Error 6: Clock must have intiger clock number greater than 0"); //9
	errorlist.push_back("Error 10: Need colon after name for CLOCK/SWITCH/GATE type"); //10
	errorlist.push_back("Error 11: Name must start with letter and only contain letter number and '_'"); //11
	errorlist.push_back("Error 12: Connection must start with the name of a device"); //12
	errorlist.push_back("Error 13: connection must start with the name of a device or end of device list must be terminated with END (not semicolon)"); //13
	errorlist.push_back("Error 14: Expect a dot after DTYPE"); //14
	errorlist.push_back("Error 15: Input device called in connection list does not exist"); //15
	errorlist.push_back("Error 16: Must specify output to connect to input with equals sign "); //16
	errorlist.push_back("Error 17: Must specify valid input gate after dot"); //17
	errorlist.push_back("Error 18: Need to specify valid iput gate seperated from device name by a '.'"); //18
	errorlist.push_back("Error 19: Output device called in connection list does not exist"); //19
	errorlist.push_back("Error 20: Monitor must start with the name of a valid device"); //20
	errorlist.push_back("Error 21: Monitor must start with the name of a device or end of device list must be terminated with END (not semicolon)"); //21
	errorlist.push_back("Error 22: Expect a dot after DTYPE as must specify output to monitor in monitor list"); //22
	errorlist.push_back("Error 23: Bad device monitor"); //23
	
	errorCount = 0;
	warningCount = 0;
}

void error::newError(int errorCode){
	cout << errorlist[errorCode] << endl;
	errorCount ++;
}

void error::newWarning(int warningCode){
	warningCount ++;
}
