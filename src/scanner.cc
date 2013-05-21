#include <iostream>
#include "scanner.h"

using namespace std;

scanner::scanner(names* names_mod, const char* defname)
{
	defnames = names_mod;
	inf.open(defname);	//Open file
	if (!inf) {
		cout << "Error: cannot open file for reading" << endl;
		exit(1);
	}
	eofile = (inf.get(curch) == 0);	//Get first character
}

scanner::~scanner()
{
	inf.close(); 	//Close file
}

void scanner::getsymbol(symbol& s, name& id, int& num)
{
	skipspaces();
	skipcomments();
	if (eofile) s = eofsym;
	else {
		if (isdigit(curch)) {
			s = numsym;
			getnumber(num);
		}
		else {
			if(isalpha(curch) {
				getname(id)
				if (id == 0) s = devsym; else
				if (id == 1) s = consym; else
				if (id == 2) s = monsym; else
				if (id == 3) s = endsym; else
				if (id > 3 && id < 12) s = classsym; else
				if (if > 11 && id < 34) s = iosym; else
				s = namesym
			}
			else {
				switch (curch) {
					case '=': s = equals; break;
					case ';': s = semicol; break;
					case ':': s = colon; break;
					case '.': s = dot; break;
					default: s = badsym; break;
				}
			getch();
			}
		}
	}
}

void scanner::getcurrentline()
{

}

void scanner::getch()
{
	eofile = (inf.get(curch) == 0);	//get next character
}

void scanner::getnumber(int& number)
{

}

void scanner::getname(name& id)
{
	namestring str;
	
	while (isalnum(curch)) { 
		str.push_back(curch);
		getch();	
	}

	if (str.size() > maxlength) {
		cout << "Warning: name '" << str << "' was truncated." << endl;
		str.resize(maxlength);
	}	
	id = defnames->lookup(str);
}

void scanner::skipspaces()
{
	while (isspace(curch)) {
		eofile = (inf.get(curch) == 0);	//get next character
		if (eofile) break;
	}
 }
 
 void scanner::skipcomments()
 {
 
 }