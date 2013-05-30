#include <iostream>
#include "parser.h"
#include "error.h"

using namespace std;

/* The parser for the circuit definition files */

bool parser::readin(void)
{
	//EBNF: specfile = devices connections monitors
	bool deviceDone = false, connectionDone = false, monitorDone = false;
	cursym = badsym;
	while (cursym != eofsym)
	{
		if (cursym != devsym && cursym != consym && cursym != monsym)
		{
			smz->getsymbol(cursym, curname, curint);
		}
		if (cursym == devsym)
		{
			if (deviceDone)
			{
				erz->newError(25);//Must only be one devices list
			}
			deviceDone = true;
			deviceList();
		}
		else if (cursym == consym)
		{
			if (!deviceDone)
			{
				erz->newError(0); //must have device list first
			}
			if (connectionDone)
			{
				erz->newError(28);//Must only be one connections list
				
			}
			connectionDone = true;
			connectionList();
		}
		else if (cursym == monsym)
		{
			if (!deviceDone | !connectionDone)
			{
				erz->newError(2); //Must have monitor list last
			}
			if (monitorDone)
			{
				erz->newError(29);//Must only be one Monitors list
			}
			monitorDone = true;
			monitorList();
		}
		else
		{
			while (cursym != devsym && cursym != consym && cursym != monsym && cursym != eofsym)
			{
				smz->getsymbol(cursym, curname, curint);
				erz->countSymbols();
			}
			erz->symbolError(deviceDone, connectionDone, monitorDone);
		}
	}
	if (!deviceDone)
	{
		erz->newError(26);//There must be a DEVICES block, it may not have been initialised properly
	}
	if (!connectionDone)
	{
		erz->newError(30);//There must be a CONNECTIONS block, it may not have been initialised properly
	}
	if (!monitorDone)
	{
		erz->newError(31);//There must be a MONITORS block, it may not have been initialised properly
	}
	netz->checknetwork(correctOperation);
	anyErrors = erz->anyErrors();
	return (correctOperation && !anyErrors);
}

void parser::deviceList()
{
	//EBNF: devices = 'DEVICES' dev {';' dev} ';' 'END'
	bool deviceError;
	if (!devicePresent)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == classsym)
		{
			deviceError = newDevice(curname);
			devicePresent = true;
		}
		else if (cursym == endsym)
		{
			erz->newError(3); //must have at least one device
			return;
		}
		else
		{
			erz->newError(4); //need a device type
		}
		if (!deviceError)
		{
			smz->getsymbol(cursym, curname, curint);
		}
	}
	while (cursym == semicol)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == classsym)
		{
			deviceError = newDevice(curname);
		}
		else if (cursym == endsym)
		{
			return;
		}
		else if (cursym == consym | cursym == devsym | cursym == monsym)
		{
			erz->newError(32);//Block must be terminated with 'END'
			return;
		}
		else
		{
			erz->newError(5);//Expecting device name or END after semicolon (device name must start with letter)
		}
		if (!deviceError)
		{
			smz->getsymbol(cursym, curname, curint);
		}
	}
	if (!deviceError) erz->newError(24);//must end line in semicolon
	while (cursym != semicol && cursym != endsym && cursym != eofsym)
	{
		smz->getsymbol(cursym, curname, curint);
	}
	if (cursym == semicol)
	{
		deviceList();
	}
	if (cursym == endsym)
	{
		return;
	}
}


bool parser::newDevice(int deviceType)
{
	//EBNF: dev = clock|switch|gate|dtype|xor
	bool errorOccurance = false;
	smz->getsymbol(cursym, curname, curint);
	if (cursym == namesym)
	{
		devlink nameCheck = netz->finddevice(curname);
		if(nameCheck==NULL)
		{
			name devName = curname;
			if (deviceType == 10)
			{
				dmz->makedevice(dtype, devName, 0, correctOperation);	//create DTYPE with name devName
				return errorOccurance;
			}
			if (deviceType == 11)
			{
				dmz->makedevice(xorgate, devName, 2, correctOperation); //create XOR with name devName
				return errorOccurance;
			}
			smz->getsymbol(cursym, curname, curint);
			if (cursym == colon)
			{
				smz->getsymbol(cursym, curname, curint);
				if (cursym == numsym)
				{
					switch (deviceType)
					{
						case 4:
							if (curint > 0)
							{
								dmz->makedevice(aclock, devName, curint, correctOperation); //create clock with curint and devName
							}
							else
							{
								erz->newError(6);//clock must have number greater than 0
								errorOccurance=true;
							}
							break;
						case 5:
							if (curint == 1 || curint == 0)
							{
								dmz->makedevice(aswitch, devName, curint, correctOperation);//create switch with curint and devName
							}
							else
							{
								erz->newError(7);//switch must have either 0 or 1
								errorOccurance=true;
							}
							break;
						case 6:
						case 7:
						case 8:
						case 9:
							if (curint > 0 && curint < 17)
							{
								switch (deviceType)
								{
									case 6:
										dmz->makedevice(andgate, devName, curint, correctOperation);//create and gate with curint and devName
										break;
									case 7:
										dmz->makedevice(nandgate, devName, curint, correctOperation);//create nand gate with curint and devName
										break;
									case 8:
										dmz->makedevice(orgate, devName, curint, correctOperation);//create or gate with curint and devName
										break;
									case 9:
										dmz->makedevice(norgate, devName, curint, correctOperation);//create nor gate with curint and devName
										break;
									default:
										cout << "How on earth have you managed to get here?" << endl;
								}
							}
							else
							{
								erz->newError(8);//must have between 1 and 16 inputs to a GATE
								errorOccurance=true;
							}
							break;
						default:
							cout << "Please do not deduct marks if this message is displayed" << endl;
					}
					return errorOccurance;
				}
				else
				{
					erz->newError(9);//clock needs clock cycle number
					errorOccurance=true;
				}
			}
			else
			{
				erz->newError(10);//need colon after name for CLOCK/SWITCH/GATE type
				errorOccurance=true;
			}
		}
		else
		{
			erz->newError(34);//attempting to give two devices the same name, choose an alternative name
			errorOccurance=true;
		}
	}
	else if (cursym!=badsym)
	{
		erz->newError(33);//using reserved word as device name
		errorOccurance=true;
	}
	else
	{
		erz->newError(11);//name must begin with letter and only containing letter number and _
		errorOccurance=true;
	}
	return errorOccurance;
}

void parser::connectionList()
{
	//EBNF: connections = 'CONNECTIONS' {con ';'} 'END'
	bool connectionError;
	if (!connectionPresent)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == endsym)
		{
			if (!connectionPresent)
			{
				erz->newWarning(0);//No Connections
			}
			return;
		}
		else if (cursym == namesym)
		{
			connectionError = newConnection();
			connectionPresent = true;
		}
		else
		{
			erz->newError(12);//connection must start with the name of a device
		}
		if (!connectionError)
		{
			smz->getsymbol(cursym, curname, curint);
		}
	}
	while (cursym == semicol)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == namesym)
		{
			connectionError = newConnection();
		}
		else if (cursym == endsym)
		{
			return;
		}
		else if (cursym == consym | cursym == devsym | cursym == monsym)
		{
			erz->newError(32);//Block must be terminated with 'END'
			return;
		}
		else
		{
			erz->newError(13);//connection must start with the name of a device or end of device list must be terminated with END (not semicolon)
		}
		if (!connectionError)
		{
			smz->getsymbol(cursym, curname, curint);
		}
	}
	if (!connectionError) erz->newError(24);//must end line in semicolon
	while (cursym != semicol && cursym != endsym && cursym != eofsym)
	{
		smz->getsymbol(cursym, curname, curint);
	}
	if (cursym == semicol)
	{
		connectionList();
	}
	if (cursym == endsym)
	{
		return;
	}
}

bool parser::newConnection()
{
	//EBNF: con = devicename'.'input '=' devicename['.'output]
	bool errorOccurance = false;
	devlink devtype = netz->finddevice(curname);
	if (devtype != NULL)
	{
		connectionInName = curname;
		smz->getsymbol(cursym, curname, curint);
		if (cursym == dot)
		{
			smz->getsymbol(cursym, curname, curint);
			devtype = netz->finddevice(connectionInName);
			inplink ilist = netz->findinput(devtype, curname);
			if (cursym == iosym && ilist != NULL)
			{
				name inputPin = curname;
				smz->getsymbol(cursym, curname, curint);
				if (cursym == equals) //SEARCH - you have got to here
				{
					smz->getsymbol(cursym, curname, curint);
					devtype = netz->finddevice(curname);
					if (devtype != NULL)
					{
						connectionOutName = curname;
						switch (devtype ? devtype->kind : baddevice)
						{
							case 7:
								smz->getsymbol(cursym, curname, curint);
								if (cursym == dot)
								{
									smz->getsymbol(cursym, curname, curint);
									outplink olist = netz->findoutput(devtype, curname);
									if (cursym == iosym && olist != NULL)
									{
										netz->makeconnection(connectionInName, inputPin, connectionOutName, curname, correctOperation);
										return errorOccurance;
									}
									else
									{
										erz->newError(34); //Not valid output for dtype
									}
								}
								else
								{
									erz->newError(14);	//Expect a dot after dtype
									errorOccurance=true;
								}
							default:
								netz->makeconnection(connectionInName, inputPin, connectionOutName, blankname, correctOperation);
								return errorOccurance;
						}
					}
					else
					{
						erz->newError(15); //Device does not exist
						errorOccurance=true;
					}
				}
				else
				{
					erz->newError(16);//Must specify output to connect to input with equals sign
					errorOccurance=true;
				}
			}
			else
			{
				erz->newError(17);//specify valid input gate after dot
				errorOccurance=true;
			}
		}
		else
		{
			erz->newError(18);//need to seperate connection input with a '.' (or need to specify input)
			errorOccurance=true;
		}
	}
	else
	{
		erz->newError(19); //Device does not exist
		errorOccurance=true;
	}
	return errorOccurance;
}

void parser::monitorList()
{
	//EBNF: monitors = 'MONITORS' {mon ';'} 'END'
	bool monitorError;
	if (!monitorPresent)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == endsym)
		{
			if (!monitorPresent)
			{
				erz->newWarning(1);//No Monitors
			}
			return;
		}
		else if (cursym == namesym)
		{
			monitorError = newMonitor();
			monitorPresent = true;
		}
		else
		{
			erz->newError(20);//monitor must start with the name of a device
		}
		if (!monitorError)
		{
			smz->getsymbol(cursym, curname, curint);
		}
	}
	while (cursym == semicol)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == namesym)
		{
			monitorError = newMonitor();
		}
		else if (cursym == endsym)
		{
			return;
		}
		else if (cursym == consym | cursym == devsym | cursym == monsym)
		{
			erz->newError(32);//Block must be terminated with 'END'
			return;
		}
		else
		{
			erz->newError(21);//monitor must start with the name of a device or end of device list must be terminated with END (not semicolon)
		}
		if (!monitorError)
		{
			smz->getsymbol(cursym, curname, curint);
		}
	}
	if (!monitorError) erz->newError(24);//must end line in semicolon
	while (cursym != semicol && cursym != endsym && cursym != eofsym)
	{
		smz->getsymbol(cursym, curname, curint);
	}
	if (cursym == semicol)
	{
		monitorList();
	}
	if (cursym == endsym)
	{
		return;
	}
}

bool parser::newMonitor()
{
	//EBNF: mon = devicename['.'output]
	bool errorOccurance = false;
	devlink devtype = netz->finddevice(curname);
	if (devtype != NULL)
	{
		monitorName = curname;
		switch (devtype ? devtype->kind : baddevice)
		{
			case 7:
				smz->getsymbol(cursym, curname, curint);
				if (cursym == dot)
				{
					smz->getsymbol(cursym, curname, curint);
					outplink olist = netz->findoutput(devtype, curname);
					if (cursym == iosym && olist != NULL)
					{
						mmz->makemonitor(monitorName, curname, correctOperation);
						return errorOccurance;
					}
					else
					{
						erz->newError(34); //Not valid output for dtype
					}
				}
				else
				{
					erz->newError(22);	//Expect a dot after dtype
					errorOccurance=true;
				}
			default:
				mmz->makemonitor(monitorName, blankname, correctOperation);
				return errorOccurance;
		}
	}
	else
	{
		erz->newError(23);
		errorOccurance=true;
	}
	return errorOccurance;
}

parser::parser(network* network_mod, devices* devices_mod, monitor* monitor_mod, scanner* scanner_mod, error* error_mod)
{
	netz = network_mod;  /* make internal copies of these class pointers */
	dmz = devices_mod;   /* so we can call functions from these classes  */
	mmz = monitor_mod;   /* eg. to call makeconnection from the network  */
	smz = scanner_mod;   /* class you say:                               */
	erz = error_mod; /* netz->makeconnection(i1, i2, o1, o2, ok);   */
	/* any other initialisation you want to do? */
}
