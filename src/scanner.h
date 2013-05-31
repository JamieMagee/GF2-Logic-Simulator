#ifndef scanner_h
#define scanner_h
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "names.h"

using namespace std;

typedef int name;
typedef enum {namesym, numsym, devsym, consym, monsym, endsym, classsym, iosym, colon, semicol, equals, dot, badsym, eofsym} symbol;

class scanner
{
	public:
		symbol s;
		names* nmz;//Pointer to instance of names class
		
		scanner(names* names_mod,		//Pointer to names class
				const char* defname,	//Name of file being read
				bool& ok);				//True of file has been opened correctly
		~scanner();						//Destructor
		void getsymbol(symbol& s,		//Symbol type read
					   name& id,			//Return symbol name (if it has one)
					   int& num,
					   string& numstring);			//Return symbol value (if it's a number)
		void writelineerror();

	private:
		ifstream inf;	//Input file
		char curch;		//Current input character
		char prevch;	//Previous input character. Used for finding line end
		bool eofile; 	//True for end of file
		bool ok;		//True if the file has been opened correctly
		int linenum;	//Number of lines in definition file
		int cursymlen;	//Length of current symbol. Used for error printing
		string line;	//Current line contents. Used for error printing

		void getch();	//Gets next input character
		void getnumber(int& number, string& numstring);	//Reads number from file
		void getname(name& id); //Reads name from file
		string getline();	//Reads the line
		void skipspaces(); //Skips spaces
		void skipcomments(); //Skips comments
};

#endif
