#include "observer.h"
#include <string>
#include <iostream>

using namespace std;

class TestClass
{
	ObserverSubject* s;
	string t;
public:
	TestClass(ObserverSubject* subj, string txt)
	{
		s = subj;
		t = txt;
		s->Attach(this,&TestClass::testfunc);
	}
	~TestClass()
	{
		s->Detach(this);
	}
	void testfunc()
	{
		cout << t << endl;
	}
};

int main(int argc, char **argv)
{
	ObserverSubject subj;
	// Create some objects and register callbacks for them in subj
	TestClass* foo = new TestClass(&subj, "foo");
	TestClass* bar = new TestClass(&subj, "bar");
	subj.Attach(foo, &TestClass::testfunc);

	// Run all callbacks
	subj.Trigger();
	/* Output:
	 * foo
	 * bar
	 * foo
	 */

	// Delete objects, and check that all callbacks have been deleted
	cout << subj.CallbackCount() << endl;
	delete foo;
	delete bar;
	cout << subj.CallbackCount() << endl;
	/* Output:
	 * 3
	 * 0
	 */

	return 0;
}

