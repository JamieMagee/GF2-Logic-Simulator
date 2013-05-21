#include "tests.h"
#include <string>
#include <iostream>
#include <vector>

using namespace std;

// Runs the set of tests indicated by testSetName
// Returns true if testSetName was valid
// If any tests failed, testsAllPassed is set to false
// Otherwise, testsAllPassed is unmodified
bool runTestSet(const string& testSetName, bool& testsAllPassed)
{
	Tests *t;
	if (testSetName=="names")
		t = new NamesTests();
	if (testSetName=="scanner")
		t = new ScannerTests();
	else
		return false;
	if (!t->runTests())
		testsAllPassed = false;
	delete t;
	return true;
}

// Exit codes: 0 = all tests successful; 1 = some tests failed; 2 = an invalid test name was given
int main(int argc, char **argv)
{
	vector<string> validNames;
	validNames.push_back("names");
	validNames.push_back("scanner");
	bool testsAllPassed = true;
	if (argc <= 1 || (argv[1] && string(argv[1])=="all"))
	{
		cout << "Running all tests" << endl;
		for (vector<string>::iterator it=validNames.begin(); it!=validNames.end(); it++)
		{
			runTestSet(*it, testsAllPassed);
		}
	}
	else
	{
		for (int i=1; i<argc; i++)
		{
			if (argv[i])
			{
				if (!runTestSet(argv[i], testsAllPassed))
				{
					cout << "Usage: " << argv[0] << " [<testnames>...]" << endl;
					cout << "Valid testnames are:";
					for (vector<string>::iterator it=validNames.begin(); it!=validNames.end(); it++)
					{
						cout << " " << *it;
					}
					cout << endl;
					if (string(argv[i]) != "help")
					{
						cout << "Invalid test set name \"" << argv[i] << "\"" << endl;
						return 2;
					}
				}
			}
		}
	}
	return !testsAllPassed;
}
