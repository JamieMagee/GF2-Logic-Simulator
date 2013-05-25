#ifndef observer_h
#define observer_h

#include <vector>
#include <iostream>
using namespace std;

class ObserverCallbackBase
{
public:
	virtual void* GetTarget() = 0;
	virtual void Run() = 0;
	virtual ~ObserverCallbackBase() {}
};

template <class T>
class ObserverCallback : public ObserverCallbackBase
{
private:
	T* targetObject;
	void (T::*targetFunc)();
public:
	ObserverCallback(T* obj, void (T::*func)())
	{
		targetObject = obj;
		targetFunc = func;
	}
	virtual void* GetTarget()
	{
		return targetObject;
	}
	virtual void Run()
	{
		(targetObject->*targetFunc)();
	}
};

class ObserverSubject
{
private:
	vector<ObserverCallbackBase*> callbacks;
	bool callingFuncs;// Whether Trigger() is currently being run
public:
	ObserverSubject() : callingFuncs(false) {}

	// Register the member function targetFunc of targetObj to be called when this ObserverSubject is triggered
	// Does not currently detect and prevent duplicates
	template <class T>
	void Attach(T* targetObj, void (T::*targetFunc)())
	{
		callbacks.push_back(new ObserverCallback<T>(targetObj, targetFunc));
	}

	// Detach all registered callbacks with the object pointer equal to targetObj
	void Detach(void* targetObj)
	{
		for (vector<ObserverCallbackBase*>::iterator it=callbacks.begin(); it<callbacks.end(); ++it)
		{
			if ((*it)->GetTarget()==targetObj)
			{
				delete *it;
				callbacks.erase(it);
			}
		}
	}

	// Runs all registered callback functions
	void Trigger()
	{
		if (callingFuncs)
		{
			// Prevent recursion
			cout << "Error: ObserverSubject::Trigger was called recursively" << endl;
			return;
		}
		callingFuncs = true;
		for (vector<ObserverCallbackBase*>::iterator it=callbacks.begin(); it<callbacks.end(); ++it)
		{
			(*it)->Run();
		}
		callingFuncs = false;
	}

	// Return the number of registered callbacks
	int CallbackCount()
	{
		return callbacks.size();
	}

	virtual ~ObserverSubject()
	{
		for (vector<ObserverCallbackBase*>::iterator it=callbacks.begin(); it<callbacks.end(); ++it)
		{
			delete *it;
		}
	}
};

/*
Example usage:
	class TestClass
	{
		ObserverSubject* s;
		string t = txt;
		TestClass(ObserverSubject* subj, string txt)
		{
			s = subj;
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


	ObserverSubject subj;
	TestClass* foo = new TestClass(&subj, "foo");
	TestClass* bar = new TestClass(&subj, "bar");
	subj.Trigger();// stuff is done, once in each test class

*/

#endif
