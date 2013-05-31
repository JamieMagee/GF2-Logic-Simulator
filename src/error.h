#ifndef error_h
#define error_h

#include "scanner.h"

typedef string errorstring;

class error
{
	private:
		int errorCount;
		int warningCount;
		int symbolCount;
		bool firstTime;
		vector<errorstring> errorlist;
		vector<errorstring> warninglist;
		scanner* smz;
	public:
		error(scanner* scanner_mod);
		void newError(int errorCode);
		void symbolError(bool deviceDone, bool connectionDone, bool monitorDone);
		void newWarning(int warningCode);
		void countSymbols();
		void monitorWarning(namestring repeatedMonitor);
		
		bool anyErrors();//outputs total number of errors and warnings and returns 1 if any errors are present
};

#endif /* error_h */
