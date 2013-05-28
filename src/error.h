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
		void countError();//counts total number of errors and 
};

#endif /* error_h */
