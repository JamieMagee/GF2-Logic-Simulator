#include <iostream>
#include "parser.h"

using namespace std;

/* The parser for the circuit definition files */


bool parser::readin (void)
{
    smz->getSymbol(cursym, curname, curint);
    if (cursym == devsym){
      deviceList();
    }
    else{
      error();
	}
    smz->getSymbol(cursym, curname, curint);
    if (cursym == consym){
      connectionList();
    }
    else{
      error();
    }
    smz->getSymbol(cursym, curname, curint);
    if (cursym == monsym){
      monitorList();
    }
    else{
      error()
	}
  }
  /* over to you */
}

void parser::deviceList(){
}

void parser::connectionList(){
}

void parser::monitorList(){
}

void parser::error()
{
  cout << "PANIC" << endl;
}

parser::parser (network* network_mod, devices* devices_mod,
		monitor* monitor_mod, scanner* scanner_mod)
{
  netz = network_mod;  /* make internal copies of these class pointers */
  dmz = devices_mod;   /* so we can call functions from these classes  */
  mmz = monitor_mod;   /* eg. to call makeconnection from the network  */
  smz = scanner_mod;   /* class you say:                               */
                       /* netz->makeconnection (i1, i2, o1, o2, ok);   */

  /* any other initialisation you want to do? */

}







