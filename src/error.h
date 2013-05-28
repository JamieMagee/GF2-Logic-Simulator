#ifndef error_h
#define error_h

#include "scanner.h"

typedef string errorstring;

class error{
	private:
		int errorCount;
		int warningCount;
		vector<errorstring> errorlist;
		scanner* smz;
	public:
		error(scanner* scanner_mod);
		void newError(int errorCode);
		void newWarning(int warningCode);
		bool anyErrors();//outputs total number of errors and warnings and returns 1 if any errors are present 
};

#endif /* error_h */
