#include "names.h"
#include <iostream>
#include <string>
#include <cstdlib>

using namespace std;

/* Name storage and retrieval routines */

names::names(void)  /* the constructor */
{
	//Populate namelist with reserved words
	namelist.push_back("DEVICES"); //0
	namelist.push_back("CONNECTIONS"); //1
	namelist.push_back("MONITORS"); //2
	namelist.push_back("END"); //3
	namelist.push_back("CLOCK"); //4
	namelist.push_back("SWITCH"); //5
	namelist.push_back("AND"); //6
	namelist.push_back("NAND"); //7
	namelist.push_back("OR"); //8
	namelist.push_back("NOR"); //9
	namelist.push_back("DTYPE"); //10
	namelist.push_back("XOR"); //11
	namelist.push_back("SIGGEN"); //12
	namelist.push_back("I1"); //13
	namelist.push_back("I2"); //14
	namelist.push_back("I3"); //15
	namelist.push_back("I4"); //16
	namelist.push_back("I5"); //17
	namelist.push_back("I6"); //18
	namelist.push_back("I7"); //19
	namelist.push_back("I8"); //20
	namelist.push_back("I9"); //21
	namelist.push_back("I10"); //22
	namelist.push_back("I11"); //23
	namelist.push_back("I12"); //24
	namelist.push_back("I13"); //25
	namelist.push_back("I14"); //26
	namelist.push_back("I15"); //27
	namelist.push_back("I16"); //28
	namelist.push_back("DATA"); //29
	namelist.push_back("CLK"); //30
	namelist.push_back("SET"); //31
	namelist.push_back("CLEAR"); //32
	namelist.push_back("Q"); //33
	namelist.push_back("QBAR"); //34
}

name names::lookup(namestring str)
{
	if (cvtname(str) == blankname)
	{
		namelist.push_back(str);	//Insert new string
		return namelist.size() - 1;	//Return new strings internal name
	}
	else
	{
		return cvtname(str);
	}
}

name names::cvtname(namestring str)
{
	if (str == "") return blankname;
	for (name id = 0; id < namelist.size(); id++)
	{
		if (namelist[id] == str) return id;		//Linear search of namelist vector
	}
	return blankname;
}

void names::writename(name id)
{
	if (id == blankname) cout << "blankname";
	else if (id > blankname && id < namelist.size()) cout << namelist[id];
	else cout << "Incorrect id";
}

int names::namelength(name id)
{
	if (id > blankname && id < namelist.size()) return namelist[id].length();
	else return blankname;
}

namestring names::getnamestring(name id)
{
	if (id > blankname && id < namelist.size()) return namelist[id];
	else return "";
}
