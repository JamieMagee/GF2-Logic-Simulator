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
		//do stuff
		cout << t << endl;
	}
};

int main(int argc, char **argv)
{
	ObserverSubject subj;
	TestClass* foo = new TestClass(&subj, "foo");
	TestClass* bar = new TestClass(&subj, "bar");
	subj.Attach(foo, &TestClass::testfunc);
	subj.Trigger();
	/* Output:
	 * foo
	 * bar
	 * foo
	 */
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

