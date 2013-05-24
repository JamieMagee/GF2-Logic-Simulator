#include <iostream>
#include "parser.h"

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
		error(); //must have device list first
	}
	smz->getsymbol(cursym, curname, curint);
	if (cursym == consym)
	{
		connectionList();
	}
	else
	{
		error(); //must have connection list second
	}
	smz->getsymbol(cursym, curname, curint);
	if (cursym == monsym)
	{
		monitorList();
	}
	else
	{
		error(); //must have monitor list last
	}
}

void parser::deviceList()
{
	//EBNF: devices = 'DEVICES' dev {';' dev} 'END'
	smz->getsymbol(cursym, curname, curint);
	if (cursym == classsym)
	{
		newDevice(curname);
	}
	else if (cursym == endsym)
	{
		error(); //must have at least one device
	}
	else
	{
		error(); //need a device type
	}
	smz->getsymbol(cursym, curname, curint);
	while (cursym == semicol)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == classsym)
		{
			newDevice(curname);
		}
		else
		{
			error();//new device must have a name or must end with END not semicolon
		}
		smz->getsymbol(cursym, curname, curint);
	}
	if (cursym == endsym)
	{
		return;
	}
	else
	{
		error();//need semicolon or END
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
			dmz->makedevice(dtype, devName, 0, ok);	//create DTYPE with name devName
			return;
		}
		if (deviceType == 11)
		{
			cout << "XOR with name integer " << devName << endl;
			dmz->makedevice(xorgate, devName, 2, ok); //create XOR with name devName
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
							dmz->makedevice(aclock, devName, curint, ok); //create clock with curint and devName
							cout << "CLOCK with name integer " << devName << " and " << curint << " program cycles per clock cycle" << endl;
						}
						else
						{
							error();//clock must have number greater than 0
						}
						break;
					case 5:
						if (curint == 1 || curint == 0)
						{
							dmz->makedevice(aswitch, devName, curint, ok);//create switch with curint and devName
							cout << "SWITCH with name integer " << devName << " and intial state " << curint << endl;
						}
						else
						{
							error();//switch must have either 0 or 1
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
									dmz->makedevice(andgate, devName, curint, ok);//create and gate with curint and devName
									cout << "AND gate with name integer " << devName << " and " << curint << " input(s)" << endl;
									break;
								case 7:
									dmz->makedevice(nandgate, devName, curint, ok);//create nand gate with curint and devName
									cout << "NAND gate with name integer " << devName << " and " << curint << " input(s)" << endl;
									break;
								case 8:
									dmz->makedevice(orgate, devName, curint, ok);//create or gate with curint and devName
									cout << "OR gate with name integer " << devName << " and " << curint << " input(s)" << endl;
									break;
								case 9:
									break;
									dmz->makedevice(norgate, devName, curint, ok);//create nor gate with curint and devName
									cout << "NOR gate with name integer " << devName << " and " << curint << " input(s)" << endl;
								default:
									cout << "How on earth have you managed to get here?" << endl;
							}
						}
						else
						{
							error();//must have between 1 and 16 inputs to a GATE
						}
						break;
					default:
						cout << "Please do not deduct marks if this message is displayed" << endl;
				}
				return;
			}
			else
			{
				error();//clock needs clock cycle number
			}
		}
		else
		{
			error();//need colon after name for CLOCK/SWITCH/GATE type
		}
	}
	else
	{
		error();//name must begin with name starting with letter and only containing letter number and _
	}
}

void parser::connectionList()
{
	//EBNF: connections = 'CONNECTIONS' [con] {';' con} 'END'
	smz->getsymbol(cursym, curname, curint);
	if (cursym == endsym)
	{
		return;
	}
	else if (cursym == namesym)
	{
		newConnection();
	}
	else
	{
		error();//connection must start with the name of a device
	}
	smz->getsymbol(cursym, curname, curint);
	while (cursym == semicol)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == namesym)
		{
			newConnection();
		}
		else
		{
			error();//connection must start with the name of a device or end of device list must be terminated with END (not semicolon)
		}
		smz->getsymbol(cursym, curname, curint);
	}
	if (cursym == endsym)
	{
		return;
	}
	else
	{
		error();//need semicolon or END
	}
}

void parser::newConnection()
{
	//EBNF: con = devicename'.'input '=' devicename['.'output]
	if (dmz->devkind (curname) != baddevice)
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
					if (dmz->devkind (curname) != baddevice)
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
										netz->makeconnection(connectionInName, inputPin, connectionOutName, cursym, ok); //DAT NESTING
										return;
									}
								}
								else
								{
									error();	//Expect a dot after dtype
								}
							default:
								netz->makeconnection(connectionInName, inputPin, connectionOutName, connectionOutName, ok);
								return;
						}
					}
					else
					{
						error(); //Device does not exist
					}
				}
				else
				{
					error();//SEARCH - you have got to here
				}
			}
			else
			{
				error();//specify input gate after dot
			}
		}
		else
		{
			error();//need to seperate connection input with a '.' (or need to specify input)
		}
	}
	else
	{
		error(); //Device does not exist
	}
}

void parser::monitorList()
{
	//EBNF: monitors = 'MONITORS' [mon] {';' mon} 'END'
	smz->getsymbol(cursym, curname, curint);
	if (cursym == endsym)
	{
		return;
	}
	else if (cursym == namesym)
	{
		newMonitor();
	}
	else
	{
		error();//monitor must start with the name of a device
	}
	smz->getsymbol(cursym, curname, curint);
	while (cursym == semicol)
	{
		smz->getsymbol(cursym, curname, curint);
		if (cursym == namesym)
		{
			newMonitor();
		}
		else
		{
			error();//monitor must start with the name of a device or end of device list must be terminated with END (not semicolon)
		}
		smz->getsymbol(cursym, curname, curint);
	}
	if (cursym == endsym)
	{
		return;
	}
	else
	{
		error();//need semicolon or END
	}
}

void parser::newMonitor()
{
	//EBNF: mon = devicename['.'output]
	smz->getsymbol(cursym, curname, curint);
	if (cursym == namesym)
	{
		if (dmz->devkind (curname) != baddevice)
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
							mmz->makemonitor(monitorName, cursym, ok);
							return;
						}
					}
					else
					{
						error();	//Expect a dot after dtype
					}
				default:
					mmz->makemonitor(monitorName, monitorName, ok);
					return;
			}
		}
		else
		{
			error();
		}
	}
}

void parser::error()
{
	cout << "PANIC" << endl;
}

parser::parser(network* network_mod, devices* devices_mod, monitor* monitor_mod, scanner* scanner_mod)
{
	netz = network_mod;  /* make internal copies of these class pointers */
	dmz = devices_mod;   /* so we can call functions from these classes  */
	mmz = monitor_mod;   /* eg. to call makeconnection from the network  */
	smz = scanner_mod;   /* class you say:                               */
	/* netz->makeconnection(i1, i2, o1, o2, ok);   */
	/* any other initialisation you want to do? */
}
