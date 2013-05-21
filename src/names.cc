#include "names.h"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

/* Name storage and retrieval routines */

names::names(void)  /* the constructor */
{
	//Populate namelist with reserved words
	namelist.push_back("DEVICES");
	namelist.push_back("CONNECTIONS");
	namelist.push_back("MONITORS");
	namelist.push_back("END");
	namelist.push_back("CLOCK");
	namelist.push_back("SWITCH");
	namelist.push_back("AND");
	namelist.push_back("NAND");
	namelist.push_back("OR");
	namelist.push_back("NOR");
	namelist.push_back("DTYPE");
	namelist.push_back("XOR");
	namelist.push_back("I1");
	namelist.push_back("I2");
	namelist.push_back("I3");
	namelist.push_back("I4");
	namelist.push_back("I5");
	namelist.push_back("I6");
	namelist.push_back("I7");
	namelist.push_back("I8");
	namelist.push_back("I9");
	namelist.push_back("I10");
	namelist.push_back("I11");
	namelist.push_back("I12");
	namelist.push_back("I13");
	namelist.push_back("I14");
	namelist.push_back("I15");
	namelist.push_back("I16");
	namelist.push_back("DATA");
	namelist.push_back("CLK");
	namelist.push_back("SET");
	namelist.push_back("CLEAR");
	namelist.push_back("Q");
	namelist.push_back("QBAR"); 
}

name names::lookup (namestring str)
{
	if (cvtname(str) == blankname) {
		namelist.push_back(str);	//Insert new string
		return namelist.size()-1;	//Return new strings internal name
	} else {
		return cvtname(str);
	}
}

name names::cvtname (namestring str)
{
	if (str == "") return blankname;
	for (name id=0; id<namelist.size(); id++) {
		if (namelist[id] == str.resize(maxlength)) return id;		//Linear search of namelist vector
	}
	return blankname;
}

void names::writename (name id)
{
	if (id == blankname) cout << "blankname" << endl;
	else if (id > blankname && id < namelist.size()) cout << namelist[id] << endl;
	else cout << "Incorrect id" << endl;
}

int names::namelength (name id)
{
	if (id > blankname && id < namelist.size()) return namelist[id].length();
	else return blankname;
}
