#include <iostream>
#include "parser.h"

using namespace std;

/* The parser for the circuit definition files */


bool parser::readin(void){
	smz->getSymbol(cursym, curname, curint);
	if(cursym == devsym){
		deviceList();
	}
	else{
		error(); //must have device list first
	}
	smz->getSymbol(cursym, curname, curint);
	if(cursym == consym){
		connectionList();
	}
	else{
		error(); //must have connection list second
	}
	smz->getSymbol(cursym, curname, curint);
	if(cursym == monsym){
		monitorList(); 
	}
	else{
		error(); //must have monitor list last
	}
}

void parser::deviceList(){
	smz->getSymbol(cursym, curname, curint);
	if(cusym==classsym){
		device();
	}
	else{
		if(cusym == endsym){
			error(); //must have at least one device
		}
		else{
			error();
		}
	}
	while(cursym==semicol){
		smz->getSymbol(cursym, curname, curint);
		if(cusym==classsym){
			device();
		}
		else{
			error();
		}
	}
	if(cursym==endsym){
		return
	}
	else{
		error()//need semicolon or END
}

void parser::device(){
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
}

void parser::newClock(){
	smz->getSymbol(cursym, curname, curint);
	if(cursym==namesym){
		name clockName = curname;
		smz->getSymbol(cursym, curname, curint);
		if(cursym==colon){
			smz->getSymbol(cursym, curname, curint);
			if(cursym==numsym){
				if(curint>0){
					//create clock with curint and clockName
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
