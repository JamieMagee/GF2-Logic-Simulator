#include <iostream>
#include "tests.h"
#include "names.h"

using namespace std;

Tests::Tests() :
	failCount(0), successCount(0), colourPrint(false), debug(false), printSuccess(false)
{
#ifdef __linux__
	colourPrint = true;
#endif
}

void Tests::testResult(string testDescription, bool result)
{
	if (result) testSucceeded(testDescription);
	else testFailed(testDescription);
}
void Tests::testFailed(string testDescription)
{
	failCount++;
	if (colourPrint) cout << "\033[31m";
	cout << "Test failed:";
	if (colourPrint) cout << "\033[0m";
	cout << " testing whether " << testDescription << endl;
}

void Tests::testSucceeded(string testDescription)
{
	successCount++;
	if (printSuccess || debug) cout << "Test passed: testing whether \"" << testDescription << endl;
}

void Tests::printResultsSummary()
{
	if (failCount)
	{
		if (colourPrint) cout << "\033[1;31m";
		cout << "FAILED: ";
		if (colourPrint) cout << "\033[0;1m";
		cout << failCount << " failed " << plural("test",failCount) << ", " << successCount << " " << plural("test",successCount) << " passed" << endl;
		if (colourPrint) cout << "\033[0m";
	}
	else
	{
		if (colourPrint) cout << "\033[1;32m";
		cout << "PASS: ";
		if (colourPrint) cout << "\033[0;1m";
		cout << "all tests successful, " << successCount << " " << plural("test",successCount) << " passed" << endl;
		if (colourPrint) cout << "\033[0m";
	}
}

bool Tests::runTests()
{
	if (title!="")
	{
		if (colourPrint) cout << "\033[1m";
		cout << "Testing " << title << ":" << endl;
		if (colourPrint) cout << "\033[0m";
	}
	tests();
	printResultsSummary();
	return (failCount==0);
}



NamesTests::NamesTests()
{
	title = "names module";
}
void NamesTests::tests()
{
	names nmz;
	name nn = nmz.cvtname("zxcv");
	if (debug) cout << "'zxcv' cvtname: " << nn << endl;
	testResult("names.cvtname returns blankname for an unknown string, when there are no entries in the table", 
		(nn == blankname));

	name n1 = nmz.lookup("abcdefgh");
	name n2 = nmz.lookup("abce");
	name n3 = nmz.lookup("abcdefgi");
	name n4 = nmz.lookup("abce");
	name n5 = nmz.lookup("abcdefghi");

	if (debug) cout << "n1=" << n1 << " n2=" << n2 << " n3=" << n3 << " n4=" << n4 << " n5=" << n5 << endl;

	testResult("names.lookup returns distinct identifiers for different strings", (n1 != n2 && n1 != n3 && n1 != n4 && n2 != n3 && n3 != n4));
	testResult("names.lookup returns the same identifier for the same string", (n2 == n4));

	nn = nmz.cvtname("abce");
	if (debug) cout << "'abce' cvtname: " << nn << endl;
	testResult("names.cvtname returns the correct identifier for 'abce'", nn==n2);

	testResult("names.lookup returns different identifiers for different strings with the same first 8 characters", (n1 != n5));

	nn = nmz.cvtname("zxcv");
	if (debug) cout << "'zxcv' cvtname: " << nn << endl;
	testResult("names.cvtname returns blankname for an unknown string, when there are entries in the table", (nn == blankname));

	testResult("names.namelength returns the correct name length for 'abce'", (nmz.namelength(n2) == 4));
	testResult("names.namelength returns the correct name length for 'abcdefgi'", (nmz.namelength(n3) == 8));

	namestring ns2 = nmz.getnamestring(n2);
	if (debug) cout << "getnamestring(n2): " << ns2 << endl;
	testResult("names.getnamestring returns the correct namestring for n2", (ns2 == "abce"));

	namestring ns5 = nmz.getnamestring(n5);
	if (debug) cout << "getnamestring(n5): " << ns5 << endl;
	testResult("names.getnamestring returns the correct namestring for n5, which is longer than 8 characters", (ns5 == "abcdefghi"));

}

ScannerTests::ScannerTests()
{
	title = "scanner module";
}

void NamesTests::tests()
{
	scanner scanz;
	
	
}
