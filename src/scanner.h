#ifndef scanner_h
#define scanner_h
#include <string>
#include <iostream>

using namespace std;

typedef int name;
typedef enum {namesym, numsym, devsym, consym, monsym, endsym, colon, semicol, equals, dot, badsym, eofsym} symbol;

class scanner {
	public:
		scanner (names* names_mod,		//Pointer to names class
				const char* defname)	//Name of file being read
				
		~scanner();		//Destructor
		
		void getSymbol(symbol& s,		//Symbol type read
					name& id,			//Return symbol name (if it has one)
					int& num);			//Return symbol value (if it's a number)
		void getCurrentLine();
	
	private:
		
};

#endif