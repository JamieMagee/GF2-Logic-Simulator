#include <iostream>
#include "parser.h"

using namespace std;

/* The parser for the circuit definition files */


bool parser::readin(void){
	smz->getsymbol(cursym, curname, curint);
	if(cursym == devsym){
		deviceList();
	}
	else{
		error(); //must have device list first
	}
	smz->getsymbol(cursym, curname, curint);
	if(cursym == consym){
		connectionList();
	}
	else{
		error(); //must have connection list second
	}
	smz->getsymbol(cursym, curname, curint);
	if(cursym == monsym){
		monitorList(); 
	}
	else{
		error(); //must have monitor list last
	}
}

void parser::deviceList(){
	smz->getsymbol(cursym, curname, curint);
	if(cursym==classsym){
		newDevice(curname);
	}
	else{
		if(cursym == endsym){
			error(); //must have at least one device
		}
		else{
			error();
		}
	}
	while(cursym==semicol){
		smz->getsymbol(cursym, curname, curint);
		if(cursym==classsym){
			newDevice(curname);
		}
		else{
			error();
		}
	}
	if(cursym==endsym){
		return;
	}
	else{
		error();//need semicolon or END
	}
}

/*void parser::device(){
	switch(curname){
		case 4:
			newClock();
			break;
		case 5:
			newSwitch();
			break;
		case 6:
		case 7:
		case 8:
		case 9:
			newGate();
			break;
		case 10:
			newDtype();
			break;
		case 11:
			newXor();
			break;
		default:
			cout << "Please do not deduct marks if this message is displayed" << endl;
	}
	return
}*/

void parser::newDevice(int deviceType){
	smz->getsymbol(cursym, curname, curint);
	if(cursym==namesym){
		name devName = curname;
		if(deviceType==10){
			cout << "DTYPE with name integer " << devName << endl;
			//create DTYPE with name devName
			return;
		}
		if(deviceType==11){
			cout << "XOR with name integer " << devName << endl;
			//create XOR with name devName
			return;
		}
		smz->getsymbol(cursym, curname, curint);
		if(cursym==colon){
			smz->getsymbol(cursym, curname, curint);
			if(cursym==numsym){
				switch(deviceType){
					case 4:
						if(curint>0){
							//create clock with curint and devName
							cout << "CLOCK with name integer " << devName << " and " << curint << " program cycles per clock cycle" << endl;
						}
						else{
							error();//clock must have number greater than 0
						}
						break;
					case 5:
						if (curint==1 | curint==0){
							//create switch with curint and devName
							cout << "SWITCH with name integer " << devName << " and intial state " << curint << endl;
						}
						else{
							error();//switch must have either 0 or 1
						}
						break;
					case 6:
					case 7:
					case 8:
					case 9:
						if(curint>0 && curint<17){
							switch(deviceType){
								case 6:
									//create and gate with curint and devName
									cout << "AND gate with name integer " << devName << " and " << curint << " input(s)" << endl;
									break;
								case 7:
									//create nand gate with curint and devName
									cout << "NAND gate with name integer " << devName << " and " << curint << " input(s)" << endl;
									break;
								case 8:
									//create or gate with curint and devName
									cout << "OR gate with name integer " << devName << " and " << curint << " input(s)" << endl;
									break;
								case 9:
									break;
									//create nor gate with curint and devName
									cout << "NOR gate with name integer " << devName << " and " << curint << " input(s)" << endl;
								default:
									cout << "How on earth have you managed to get here?" << endl;
							}
						}
						else{
							error();//must have between 1 and 16 inputs to a GATE
						}
						break;
					default:
						cout << "Please do not deduct marks if this message is displayed" << endl;
				}
				return;
			}
			else{
				error();//clock needs clock cycle number
			}
		}
		else{
			error();//need colon after name for CLOCK/SWITCH/GATE type
		}
	}
	else{
		error();//name must begin with name starting with letter and only containing letter number and _
	}
}


/*void parser::newClock(){
	smz->getsymbol(cursym, curname, curint);
	if(cursym==namesym){
		name clockName = curname;
		smz->getsymbol(cursym, curname, curint);
		if(cursym==colon){
			smz->getsymbol(cursym, curname, curint);
			if(cursym==numsym){
				if(curint>0){
					//create clock with curint and clockName
					cout << "CLOCK " << clockName << ":" << curint << endl;
				}
				else{
					error();//clock must have number greater than 0
				}
			}
			else{
				error();//clock needs clock cycle number
			}
		}
		else{
			error()//need colon after name
		}
	}
	else{
		error();//name must begin with name starting with letter and only containing letter number and _
	}
}

*/

void parser::connectionList(){
}

void parser::monitorList(){
}

void parser::error(){
	cout << "PANIC" << endl;
}

parser::parser(network* network_mod, devices* devices_mod, monitor* monitor_mod, scanner* scanner_mod){
	netz = network_mod;  /* make internal copies of these class pointers */
	dmz = devices_mod;   /* so we can call functions from these classes  */
	mmz = monitor_mod;   /* eg. to call makeconnection from the network  */
	smz = scanner_mod;   /* class you say:                               */
	/* netz->makeconnection(i1, i2, o1, o2, ok);   */

	/* any other initialisation you want to do? */

}
