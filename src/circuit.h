#ifndef circuit_h
#define circuit_h

#include "names.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"
#include "observer.h"
#include <vector>
using namespace std;

class circuit;


// Many bits of the GUI involve manipulating (creating, searching, sorting) vectors where each element needs to contain one or more pointers to circuit elements, with an associated string. This class is a unified way of doing that for all circuit elements (inputs, outputs, and devices).
class CircuitElementInfo
{
public:
	devlink d;
	outplink o;
	inplink i;
	string namestr;
	CircuitElementInfo() : d(NULL), o(NULL), i(NULL), namestr("") {}
	CircuitElementInfo(devlink dev, string str="") : d(dev), o(NULL), i(NULL), namestr(str) {}
	CircuitElementInfo(devlink dev, outplink outp, string str="") : d(dev), o(outp), i(NULL), namestr(str) {}
	CircuitElementInfo(outplink outp, string str="") : d(outp->dev), o(outp), i(NULL), namestr(str) {}
	CircuitElementInfo(devlink dev, inplink inp, string str="") : d(dev), o(NULL), i(inp), namestr(str) {}
};

class CircuitElementInfoVector : public vector<CircuitElementInfo>
{
public:
	// push_back() all devices in a linked list of devices, all outputs in a linked list of devices, or all inputs in a linked list of devices
	void push_back_all_devs(devlink d);
	void push_back_all_outputs(devlink d);
	void push_back_all_inputs(devlink d);
	// push_back() all outputs or inputs for a particular device
	void push_back_dev_outputs(devlink d);
	void push_back_dev_inputs(devlink d);
	// Sets namestr for each element to the device or signal name (e.g. S1, or G1.I1, or DT1.CLK)
	void UpdateSignalNames(circuit* c);
private:
	template <class T> void push_back_iolist(devlink d, T item);
};

bool CircuitElementInfo_namestrcmp(const CircuitElementInfo a, const CircuitElementInfo b);// to alphabetically sort a CircuitElementInfoVector
bool CircuitElementInfo_iconnect_namestrcmp(const CircuitElementInfo a, const CircuitElementInfo b);// to sort a CircuitElementInfoVector by input connected state then alphabetically by namestr


// Used in the device editing GUI to store a pointer to the currently selected device and notify widgets when it changes. This is here instead of in a gui file because it contains nothing specific to wxWidgets and may be useful elsewhere
class SelectedDevice
{
private:
	devlink selected;
public:
	ObserverSubject changed;
	devlink Get()
	{
		return selected;
	}
	void Set(devlink d)
	{
		if (d!=selected)
		{
			selected = d;
			changed.Trigger();
		}
	}
};


// This class is a more convenient way of passing around a set of names, monitors, devices, and network modules. 
// It also handles observers for changes to the circuit.
// It provides a home for a few functions that are more closely related to the circuit itself than to the GUI or MyFrame, such as running the network
class circuit
{
private:
	// Module pointers
	names *names_mod;
	devices *devices_mod;
	monitor *monitor_mod;
	network *network_mod;
	bool ownModules;
	void AllocModules();
	void DestroyModules();

	int continuedCycles;// how many simulation cycles were completed last time the run or continue button was used
	int totalCycles;// how many simulation cycles have been completed in total

public:
	circuit();
	circuit(names *existing_names_mod, devices *existing_devices_mod, monitor *existing_monitor_mod, network *existing_network_mod);
	virtual ~circuit();
	// Swap module pointers without changing observers
	void SwapModules(circuit& source);
	// clear the circuit, currently done by allocating new modules
	void Clear();
	// Clear recorded samples
	void ResetMonitors();
	// Simulate the logic circuit, optionally clearing monitors beforehand
	bool Simulate(int ncycles, bool resetBefore=true);
	// GetUnmonitoredOutputs returns true if there are unmonitored outputs in the circuit
	//   unmonitoredOutputsRet is an optional pointer to a vector to hold the list of unmonitored outputs
	bool GetUnmonitoredOutputs(CircuitElementInfoVector * unmonitoredOutputsRet=NULL);

	ObserverSubject monitorsChanged;// addition or removal of monitors
	ObserverSubject circuitChanged;// changes to devices, device properties, and connections
	ObserverSubject monitorSamplesChanged;// simulation has been run or continued, or monitor samples have been cleared

	names* nmz(){ return names_mod; }
	devices* dmz(){ return devices_mod; }
	monitor* mmz(){ return monitor_mod; }
	network* netz(){ return network_mod; }
	int GetTotalCycles(){ return totalCycles; }
	int GetContinuedCycles(){ return continuedCycles; }
	static bool IsDeviceNameValid(string devname);
	void RemoveDevice(devlink d);
};

#endif
