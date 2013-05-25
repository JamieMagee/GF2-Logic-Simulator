#ifndef circuit_h
#define circuit_h

#include "names.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"
#include "observer.h"

// This class is a more convenient way of passing around a set of names, monitors, devices, and network modules. 
// It also handles observers for changes to the circuit, and has a few wrapper functions that just call another function in one of the modules then trigger observers 
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

	ObserverSubject monitorsChanged;// change to which signals are being monitored
	ObserverSubject circuitChanged;// changes to devices, device properties, and connections
	ObserverSubject monitorSamplesChanged;// simulation has been run or continued or monitor samples have been cleared

	names* nmz(){ return names_mod; }
	devices* dmz(){ return devices_mod; }
	monitor* mmz(){ return monitor_mod; }
	network* netz(){ return network_mod; }
	int GetTotalCycles(){ return totalCycles; }
	int GetContinuedCycles(){ return continuedCycles; }
	
};


#endif
