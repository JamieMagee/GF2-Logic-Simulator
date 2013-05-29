#ifndef circuit_h
#define circuit_h

#include "names.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"
#include "observer.h"


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

bool CircuitElementInfo_namestrcmp(const CircuitElementInfo a, const CircuitElementInfo b);// to alphabetically sort a vector<outputinfo>
bool CircuitElementInfo_iconnect_namestrcmp(const CircuitElementInfo a, const CircuitElementInfo b);// to sort a vector<outputinfo> by input connected state then namestr

struct outputinfo
{
	name devname, outpname;
	outplink o;
	string namestr;
};
bool outputinfo_namestrcmp(const outputinfo a, const outputinfo b);// to alphabetically sort a vector<outputinfo>

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
	bool GetUnmonitoredOutputs(vector<outputinfo> * unmonitoredOutputsRet=NULL);

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
