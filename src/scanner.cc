#include <iostream>
#include "scanner.h"

using namespace std;

scanner::scanner(names* names_mod, const char* defname, bool& ok)
{
	nmz = names_mod;
	ok = 1;
	inf.open(defname);	//Open file
	if (!inf)
	{
		cout << "Error: cannot open file for reading" << endl;
		ok = 0;
	}
	eofile = (inf.get(curch) == 0);	//Get first character
	linenum = 1;
	s = badsym;//in case getline is called before getsymbol
}

scanner::~scanner()
{
	inf.close(); 	//Close file
}

void scanner::getsymbol(symbol& s, name& id, int& num, string& numstring)
{
	s = badsym;
	cursymlen = 0;
	skipspaces();
	if (eofile) s = eofsym;
	else
	{
		if (isdigit(curch))
		{
			s = numsym;
			getnumber(num, numstring);
		}
		else
		{
			if (isalpha(curch))
			{
				getname(id);
				if (id == 0) s = devsym;
				else if (id == 1) s = consym;
				else if (id == 2) s = monsym;
				else if (id == 3) s = endsym;
				else if (id > 3 && id < 13) s = classsym;
				else if (id > 12 && id < lastreservedname+1) s = iosym;
				else s = namesym;
			}
			else
			{
				switch (curch)
				{
					case '=':
						s = equals;
						getch();
						break;
					case ';':
						s = semicol;
						getch();
						break;
					case ':':
						s = colon;
						getch();
						break;
					case '.':
						s = dot;
						getch();
						break;
					case '/':
						getch();
						if (curch == '*')
						{
							getch();
							skipcomments();
							getsymbol(s, id, num, numstring);
						}
						break;
					default:
						s = badsym;
						getch();
						break;
				}
				cursymlen = 1;
			}
		}
	}
}

void scanner::writelineerror()
{
	string errorptr;
	for (int i = 0; i < ((int)line.length() - cursymlen); i++)
	{
		errorptr.push_back(' ');
	}
	errorptr.push_back('^');
	cout << "Line " << linenum << ":" << endl;
	cout << getline() << endl;		//Outputs current line
	cout << errorptr << endl;	//Outputs a caret at the error
}

void scanner::getch()
{
	prevch = curch;
	eofile = (inf.get(curch) == 0);	//get next character
	if (prevch == '\n') //If eoline, clear the currently stored line
	{
		linenum++;
		line.clear();
	}
	else if (prevch != '\r') //If we're not at the end of a line, add the char to the line string
	{
		line.push_back(prevch);
	}
}

void scanner::getnumber(int& number, string& numstring)
{
	numstring = "";
	number = 0;
	cursymlen = 0;
	while (isdigit(curch) && !eofile)
	{
		numstring.push_back(curch);
		number *= 10;
		number += (int(curch) - int('0'));
		cursymlen++;
		getch();
	}
}

void scanner::getname(name& id)
{
	namestring str;
	cursymlen = 0;
	while (isalnum(curch) && !eofile)
	{
		str.push_back(curch);
		cursymlen++;
		getch();
	}
	id = nmz->lookup(str);
}

void scanner::skipspaces()
{
	while (isspace(curch) || curch == '\n')
	{
		getch();
		if (eofile) break;
	}
}

void scanner::skipcomments()
{
	while (!(prevch == '*' && curch == '/'))
	{
		getch();
		if (eofile)
		{
			cout << "Reached end of file before comment was terminated" << endl;
			break;
		}
	}
	getch(); //Get to next useful char
}

string scanner::getline()
{
	if (s != semicol)
	{
		while (curch != ';' && !eofile && curch != '\n')
		{
			getch();
		}
		if (curch != '\n' && curch != '\r' && !eofile)
		{
			line.push_back(curch);
		}
	}
	return line;
}
