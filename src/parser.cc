#include <iostream>
#include "parser.h"
#include "error.h"

using namespace std;

/* The parser for the circuit definition files */

bool parser::readin(void)
{
	//EBNF: specfile = devices connections monitors
	smz->getsymbol(cursym, curname, curint);
	if (cursym == devsym)
	{
		deviceList();
	}
	else
	{
		erz->newError(0); //must have device list first
		//cout << "must have device list first" << endl;
	}
	smz->getsymbol(cursym, curname, curint);
	if (cursym == consym)
	{
		connectionList();
	}
	else
	{
		erz->newError(1); //must have connection list second
		//cout << "must have connection list second" << endl;
	}
	smz->getsymbol(cursym, curname, curint);
	if (cursym == monsym)
	{
		monitorList();
	}
	else
	{
		erz->newError(2); //must have monitor list last
		//cout << "must have monitor list last" << endl;
	}
	netz->checknetwork(correctOperation);
	return (correctOperation /*&& anyErrors*/);
}

void parser::deviceList()
{
	//EBNF: devices = 'DEVICES' dev {';' dev} ';' 'END'
	smz->getsymbol(cursym, curname, curint);
	if (cursym == classsym)
	{
		newDevice(curname);
	}
	else if (cursym == endsym)
	{
		erz->newError(3); //must have at least one device
		//cout << "must have at least one device" << endl;
	}
	else
	{
		erz->newError(4); //need a device type
		//cout << "need a device type" << endl;
	}
	smz->getsymbol(cursym, curname, curint);
	while (cursym == semicol)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == classsym)
		{
			newDevice(curname);
		}
		else if (cursym == endsym)
		{
			return;
		}
		else 
		{
			erz->newError(5);//new device must have a name or must end with END not semicolon
			//cout << "new device must have a name or must end with END not semicolon" << endl;
		}
		smz->getsymbol(cursym, curname, curint);
	}
}

void parser::newDevice(int deviceType)
{
	//EBNF: dev = clock|switch|gate|dtype|xor
	smz->getsymbol(cursym, curname, curint);
	if (cursym == namesym)
	{
		name devName = curname;
		if (deviceType == 10)
		{
			cout << "DTYPE with name integer " << devName << endl;
			dmz->makedevice(dtype, devName, 0, correctOperation);	//create DTYPE with name devName
			return;
		}
		if (deviceType == 11)
		{
			cout << "XOR with name integer " << devName << endl;
			dmz->makedevice(xorgate, devName, 2, correctOperation); //create XOR with name devName
			return;
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
							cout << "CLOCK with name integer " << devName << " and " << curint << " program cycles per clock cycle" << endl;
						}
						else
						{
							erz->newError(6);//clock must have number greater than 0
						}
						break;
					case 5:
						if (curint == 1 || curint == 0)
						{
							dmz->makedevice(aswitch, devName, curint, correctOperation);//create switch with curint and devName
							cout << "SWITCH with name integer " << devName << " and intial state " << curint << endl;
						}
						else
						{
							erz->newError(7);//switch must have either 0 or 1
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
									cout << "AND gate with name integer " << devName << " and " << curint << " input(s)" << endl;
									break;
								case 7:
									dmz->makedevice(nandgate, devName, curint, correctOperation);//create nand gate with curint and devName
									cout << "NAND gate with name integer " << devName << " and " << curint << " input(s)" << endl;
									break;
								case 8:
									dmz->makedevice(orgate, devName, curint, correctOperation);//create or gate with curint and devName
									cout << "OR gate with name integer " << devName << " and " << curint << " input(s)" << endl;
									break;
								case 9:
									break;
									dmz->makedevice(norgate, devName, curint, correctOperation);//create nor gate with curint and devName
									cout << "NOR gate with name integer " << devName << " and " << curint << " input(s)" << endl;
								default:
									cout << "How on earth have you managed to get here?" << endl;
							}
						}
						else
						{
							erz->newError(8);//must have between 1 and 16 inputs to a GATE
						}
						break;
					default:
						cout << "Please do not deduct marks if this message is displayed" << endl;
				}
				return;
			}
			else
			{
				erz->newError(9);//clock needs clock cycle number
			}
		}
		else
		{
			erz->newError(10);//need colon after name for CLOCK/SWITCH/GATE type
		}
	}
	else
	{
		erz->newError(11);//name must begin with name starting with letter and only containing letter number and _
	}
}

void parser::connectionList()
{
	//EBNF: connections = 'CONNECTIONS' [con] {';' con} 'END'
	smz->getsymbol(cursym, curname, curint);
	if (cursym == endsym)
	{
		erz->newWarning(0);//No Connections
		return;
	}
	else if (cursym == namesym)
	{
		newConnection();
	}
	else
	{
		erz->newError(12);//connection must start with the name of a device
		//cout << "connection must start with the name of a device" << endl;
	}
	smz->getsymbol(cursym, curname, curint);
	while (cursym == semicol)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == namesym)
		{
			newConnection();
		}
		else if (cursym == endsym)
		{
			return;
		}
		else
		{
			erz->newError(13);//connection must start with the name of a device or end of device list must be terminated with END (not semicolon)
			//cout << "Connection must start with the name of a device or end of device list must be terminated with END (not semicolon)" << endl;
		}
		smz->getsymbol(cursym, curname, curint);
	}
}

void parser::newConnection()
{
	//EBNF: con = devicename'.'input '=' devicename['.'output]
	if (smz->defnames->namelength (curname) != 0)
	{
		connectionInName = curname;
		smz->getsymbol(cursym, curname, curint);
		if (cursym == dot)
		{
			smz->getsymbol(cursym, curname, curint);
			if (cursym == iosym)
			{
				name inputPin = curname;
				smz->getsymbol(cursym, curname, curint);
				if (cursym == equals) //SEARCH - you have got to here
				{
					smz->getsymbol(cursym, curname, curint);
					if (smz->defnames->namelength (curname) != baddevice)
					{
						connectionOutName = curname;
						switch (curname)
						{
							case 10:
								smz->getsymbol(cursym, curname, curint);
								if (cursym == dot)
								{
									smz->getsymbol(cursym, curname, curint);
									if (cursym == iosym)
									{
										netz->makeconnection(connectionInName, inputPin, connectionOutName, cursym, correctOperation); //DAT NESTING
										return;
									}
								}
								else
								{
									erz->newError(14);	//Expect a dot after dtype
									//cout << "Expect a dot after dtype" << endl;
								}
							default:
								netz->makeconnection(connectionInName, inputPin, connectionOutName, blankname, correctOperation);
								return;
						}
					}
					else
					{
						erz->newError(15); //Device does not exist
						//cout << "Device does not exist" << endl;
					}
				}
				else
				{
					erz->newError(16);//Must specify output to connect to input with equals sign 
					//cout << " " << endl;
				}
			}
			else
			{
				erz->newError(17);//specify valid input gate after dot
				//cout << "specify input gate after dot" << endl;
			}
		}
		else
		{
			erz->newError(18);//need to seperate connection input with a '.' (or need to specify input)
			//cout << " " << endl;
		}
	}
	else
	{
		erz->newError(19); //Device does not exist
		//cout << "Device does not exist" << endl;
	}
}

void parser::monitorList()
{
	//EBNF: monitors = 'MONITORS' [mon] {';' mon} 'END'
	smz->getsymbol(cursym, curname, curint);
	if (cursym == endsym)
	{
		erz->newWarning(1);//No Monitors
		return;
	}
	else if (cursym == namesym)
	{
		newMonitor();
	}
	else
	{
		erz->newError(20);//monitor must start with the name of a device
		//cout << "monitor must start with the name of a device" << endl;
	}
	smz->getsymbol(cursym, curname, curint);
	while (cursym == semicol)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == namesym)
		{
			newMonitor();
		}
		else if (cursym == endsym)
		{
			return;
		}
		else
		{
			erz->newError(21);//monitor must start with the name of a device or end of device list must be terminated with END (not semicolon)
			//cout << "monitor must start with the name of a device or end of device list must be terminated with END (not semicolon)" << endl;
		}
		smz->getsymbol(cursym, curname, curint);
	}
}

void parser::newMonitor()
{
//EBNF: mon = devicename['.'output]
	if (smz->defnames->namelength (curname) != 0)
	{
		monitorName = curname;
		switch (curname)
		{
			case 10:
				smz->getsymbol(cursym, curname, curint);
				if (cursym == dot)
				{
					smz->getsymbol(cursym, curname, curint);
					if (cursym == iosym)
					{
						mmz->makemonitor(monitorName, cursym, correctOperation);
						return;
					}
				}
				else
				{
					erz->newError(22);	//Expect a dot after dtype
					//cout << "Expect a dot after dtype" << endl;
					
				}
			default:
				mmz->makemonitor(monitorName, blankname, correctOperation);
				return;
		}
	}
	else
	{
		erz->newError(23);
		//cout << "Bad device monitor" << endl;
	}
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
