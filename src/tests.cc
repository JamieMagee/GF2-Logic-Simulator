#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
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
	cout << " " << testDescription << endl;
}

void Tests::testSucceeded(string testDescription)
{
	successCount++;
	if (printSuccess || debug) cout << "Test passed: " << testDescription << endl;
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

string scannerSymStrings[] = {"namesym", "numsym", "devsym", "consym", "monsym", "endsym", "classsym", "iosym", "colon", "semicol", "equals", "dot", "badsym", "eofsym"};

void ScannerTests::tests()
{
	vector<ScannerExpectSym> expected;
	string inputTxt;
	
	inputTxt = "\
DEVICES \n\
SWITCH S1 : 0 ;\n\
END \n\
";
	expected.resize(0);
	expected.push_back(ScannerExpectSym(__LINE__, devsym));
	expected.push_back(ScannerExpectSym(__LINE__, classsym, "SWITCH", 0, SCANEXPECT_MATCH_TXT));
	expected.push_back(ScannerExpectSym(__LINE__, namesym, "S1", 0, SCANEXPECT_MATCH_TXT));
	expected.push_back(ScannerExpectSym(__LINE__, colon));
	expected.push_back(ScannerExpectSym(__LINE__, numsym, "", 0, SCANEXPECT_MATCH_NUM));
	expected.push_back(ScannerExpectSym(__LINE__, semicol));
	expected.push_back(ScannerExpectSym(__LINE__, endsym));
	expected.push_back(ScannerExpectSym(__LINE__, eofsym));
	checkSyms("test 1", inputTxt, expected);

	inputTxt = "\
DEVICES\n\
SWITCH S1:0;\n\
  /* test */END \n\
/* test */    /* test2 */CONNECTIONS\n\
END\n\
/ \n\
";
	expected.resize(0);
	expected.push_back(ScannerExpectSym(__LINE__, devsym));
	expected.push_back(ScannerExpectSym(__LINE__, classsym, "SWITCH", 0, SCANEXPECT_MATCH_TXT));
	expected.push_back(ScannerExpectSym(__LINE__, namesym, "S1", 0, SCANEXPECT_MATCH_TXT));
	expected.push_back(ScannerExpectSym(__LINE__, colon));
	expected.push_back(ScannerExpectSym(__LINE__, numsym, "", 0, SCANEXPECT_MATCH_NUM));
	expected.push_back(ScannerExpectSym(__LINE__, semicol));
	expected.push_back(ScannerExpectSym(__LINE__, endsym));
	expected.push_back(ScannerExpectSym(__LINE__, consym));
	expected.push_back(ScannerExpectSym(__LINE__, endsym));
	expected.push_back(ScannerExpectSym(__LINE__, badsym));
	expected.push_back(ScannerExpectSym(__LINE__, eofsym));
	checkSyms("test 2", inputTxt, expected);

	inputTxt = "\
DEVICES \n\
SWITCH S1 : 0 ;\n\
END\n\
";
	expected.resize(0);
	expected.push_back(ScannerExpectSym(__LINE__, devsym));
	expected.push_back(ScannerExpectSym(__LINE__, classsym, "SWITCH", 0, SCANEXPECT_MATCH_TXT));
	expected.push_back(ScannerExpectSym(__LINE__, namesym, "S1", 0, SCANEXPECT_MATCH_TXT));
	expected.push_back(ScannerExpectSym(__LINE__, colon));
	expected.push_back(ScannerExpectSym(__LINE__, numsym, "", 0, SCANEXPECT_MATCH_NUM));
	expected.push_back(ScannerExpectSym(__LINE__, semicol));
	expected.push_back(ScannerExpectSym(__LINE__, endsym));
	expected.push_back(ScannerExpectSym(__LINE__, eofsym));
	checkSyms("test 3", inputTxt, expected);

	inputTxt = "CONNECTIONS";
	expected.resize(0);
	expected.push_back(ScannerExpectSym(__LINE__, consym));
	checkSyms("single word", inputTxt, expected);

	inputTxt = "12345";
	expected.resize(0);
	expected.push_back(ScannerExpectSym(__LINE__, numsym, "", 12345, SCANEXPECT_MATCH_NUM));
	checkSyms("single number", inputTxt, expected);
}

void ScannerTests::checkSyms(string testDescription, string inputTxt, vector<ScannerExpectSym>& expected)
{
	string fileName = "scannertest.tmp.gf2";
	ofstream f;
	f.open(fileName.c_str(), ios::out | ios::trunc);
	f << inputTxt;
	f.close();

	names *nmz = new names();
	scanner *smz = new scanner(nmz, fileName.c_str());
	bool ok = true;
	symbol sym;
	name id;
	int num;
	for (int i=0; i<expected.size(); i++)
	{
		id = blankname;
		num = 0;
		smz->getsymbol(sym, id, num);
		if (!expected[i].matches(nmz, sym, id, num))
		{
			ok = false;
			testFailed(testDescription);
		}
		if (debug || !ok)
		{
			expected[i].write();
			cout << "Actual symbol: " << scannerSymStrings[sym] << ", text '" << nmz->getnamestring(id) << "', num " << num << endl;
		}
		if (!ok) break;
	}
	if (ok) testSucceeded(testDescription);

	remove(fileName.c_str());
}

ScannerExpectSym::ScannerExpectSym(int lineNum_in, symbol sym, string txt, int num, int flags_in) :
	lineNum(lineNum_in), expectedSym(sym), expectedTxt(txt), expectedNum(num), flags(flags_in)
{}

bool ScannerExpectSym::matches(names *nmz, symbol sym, name id, int num)
{
	if (expectedSym!=sym)
	{
		return false;
	}
	if (flags & SCANEXPECT_MATCH_TXT)
	{
		if (nmz->cvtname(expectedTxt) != id)
			return false;
	}
	if (flags & SCANEXPECT_MATCH_NUM)
	{
		if (expectedNum != num)
			return false;
	}
	return true;
}

void ScannerExpectSym::write()
{
	cout << "from tests line " << lineNum << ": expected symbol ";
	cout << scannerSymStrings[expectedSym];
	if (flags & SCANEXPECT_MATCH_TXT)
		cout << " with text '" << expectedTxt << "'";
	if (flags & SCANEXPECT_MATCH_NUM)
		cout << " with num " << expectedNum;
	cout << endl;
}
