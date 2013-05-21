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

class scanner {
	public:
		symbol s;
		
		scanner (names* names_mod,		//Pointer to names class
				const char* defname);	//Name of file being read
		~scanner();						//Destructor
		void getsymbol(symbol& s,		//Symbol type read
					name& id,			//Return symbol name (if it has one)
					int& num);			//Return symbol value (if it's a number)
		void getcurrentline();
	
	private:
		ifstream inf;	//Input file
		char curch;		//Current input character
		bool eofile; 	//True for end of file
		names *defnames;

		void getch();	//Gets next input character
		void getnumber(int& number);	//Reads number from file
		void getname(name& id); //Reads name from file	
		void skipspaces(); //Skips spaces
		void skipcomments(); //Skips comments
};

#endif
