#ifndef tests_h
#define tests_h

#include "scanner.h"
#include <string>

using namespace std;

class Tests
{
private:
	int failCount;
	int successCount;
	string plural(string base, int i) { return i==1?base:base+"s"; }
	void printTitle();
	void printResultsSummary();
protected:
	// Title of the thing being tested, e.g. "names module"
	string title;
	// Functions to increment the success/fail count, and print descriptions for failed tests
	void testFailed(string testDescription);
	void testSucceeded(string testDescription);
	void testResult(string testDescription, bool result);
public:
	Tests();
	virtual ~Tests(){};
	// Whether to use Linux console colour codes in the output, to highlight failed tests
	bool colourPrint;
	// Whether to produce verbose output about the values checked by the tests
	bool debug;
	// Whether to print descriptions of passed tests (automatically turned on by debug)
	bool printSuccess;
	// Abstract virtual function which should be overidden in a derived class and actually run the tests
	virtual void tests() = 0;
	// A wrapper for tests() which prints a title and summary, and returns a bool indicating whether all tests passed
	bool runTests();
};

class NamesTests : public Tests
{
public:
	NamesTests();
	virtual void tests();
};


const int SCANEXPECT_MATCH_TXT = 1;
const int SCANEXPECT_MATCH_NUM = 2;
class ScannerExpectSym
{
private:
	int lineNum;//Line in tests.cc where this object was constructed, to make it easier to determine exactly which bit of the test is failing
	symbol expectedSym;
	string expectedTxt;
	int expectedNum, flags;
public:
	ScannerExpectSym(int lineNum_in=0, symbol sym=badsym, string txt="", int num=0, int flags_in=0);
	bool matches(names *nmz, symbol sym, name id, int num);
	void write();
};

class ScannerTests : public Tests
{
public:
	ScannerTests();
	virtual void tests();
	void checkSyms(string testDescription, string inputTxt, vector<ScannerExpectSym>& expected);
};

#endif /* tests_h */
