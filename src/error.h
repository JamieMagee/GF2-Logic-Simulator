#ifndef error_h
#define error_h

#include "names.h"
#include "scanner.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"
#include "parser.h"

typedef string errorstring;

class error{
	private:
		int errorCount;
		int warningCount;
		vector<errorstring> errorlist;	
	public:
		error();
		void newError(int errorCode);
		void newWarning(int warningCode);
		void countError();//counts total number of errors and 
};

#endif /* error_h */
