#include <iostream>
#include "scanner.h"

using namespace std;

scanner::scanner(names* names_mod, const char* defname)
{
	defnames = names_mod;
	inf.open(defname);	//Open file
	if (!inf)
	{
		cout << "Error: cannot open file for reading" << endl;
	}
	eofile = (inf.get(curch) == 0);	//Get first character
	linenum=1;
}

scanner::~scanner()
{
	inf.close(); 	//Close file
}

void scanner::getsymbol(symbol& s, name& id, int& num)
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
			getnumber(num);
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
				else if (id > 3 && id < 12) s = classsym;
				else if (id > 11 && id < 34) s = iosym;
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
							getsymbol(s, id, num);
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
	for (int i = 0; i < (line.length() - cursymlen); i++)
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
	if (curch == '\n')
	{
		linenum++;
		line.clear();
	}
	if (prevch != '\n')
	{
		line.push_back(prevch);
	}
}

void scanner::getnumber(int& number)
{
	number = 0;
	cursymlen = 0;
	while (isdigit(curch) && !eofile)
	{
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
	id = defnames->lookup(str);
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
		if (eofile) break;
	}
	getch(); //Get to next useful char
}

string scanner::getline()
{
	if (s != semicol)
	{
		while (curch != ';' && !eofile)
		{
			getch();
		}
		line.push_back(curch);
	}
	return line;
}
