#ifndef tests_h
#define tests_h

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


#endif /* tests_h */
