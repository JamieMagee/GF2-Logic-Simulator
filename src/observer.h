#ifndef observer_h
#define observer_h

#include <vector>
#include <iostream>
using namespace std;

// Abstract base class for storing callback functions
class ObserverCallbackBase
{
public:
	virtual void* GetTarget() = 0;
	virtual void Run() = 0;
	virtual ~ObserverCallbackBase() {}
};

// Derived class for storing callbacks, using templates to allow member functions in different classes of the form "void SomeClass:func()"
// to be called without needing a different derived ObserverCallback class for each SomeClass
template <class T>
class ObserverCallback : public ObserverCallbackBase
{
private:
	T* targetObject;//pointer to the object which will have a member function called
	void (T::*targetFunc)();//pointer to the member function to call
public:
	ObserverCallback(T* obj, void (T::*func)())
	{
		targetObject = obj;
		targetFunc = func;
	}
	virtual void* GetTarget()
	{
		// Return the pointer to the target object, used in ObserverSubject.Detach() to check whether this callback is for the object being detached
		return targetObject;
	}
	virtual void Run()
	{
		// Run the callback function
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

// Example usage in observer-test.cc

#endif
