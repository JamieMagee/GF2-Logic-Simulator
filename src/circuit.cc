#include "circuit.h"
#include <algorithm>
using namespace std;

circuit::circuit()
{
	AllocModules();
	totalCycles = continuedCycles = 0;
}

circuit::circuit(names *existing_names_mod, devices *existing_devices_mod, monitor *existing_monitor_mod, network *existing_network_mod)
{
	names_mod = existing_names_mod;
	devices_mod = existing_devices_mod;
	monitor_mod = existing_monitor_mod;
	network_mod = existing_network_mod;
	ownModules = false;
	totalCycles = continuedCycles = 0;
}

circuit::~circuit()
{
	DestroyModules();
}

void circuit::AllocModules()
{
	names_mod = new names();
	network_mod = new network(names_mod);
	devices_mod = new devices(names_mod, network_mod);
	monitor_mod = new monitor(names_mod, network_mod);
	ownModules = true;
}

void circuit::DestroyModules()
{
	if (ownModules)
	{
		delete monitor_mod;
		delete devices_mod;
		delete network_mod;
		delete names_mod;
	}
	monitor_mod = NULL;
	devices_mod = NULL;
	network_mod = NULL;
	names_mod = NULL;
	ownModules = false;
}

void circuit::Clear()
{
	DestroyModules();
	AllocModules();
	totalCycles = continuedCycles = 0;
	circuitChanged.Trigger();
	monitorsChanged.Trigger();
	monitorSamplesChanged.Trigger();
}

void circuit::SwapModules(circuit& source)
{
	swap(names_mod, source.names_mod);
	swap(devices_mod, source.devices_mod);
	swap(monitor_mod, source.monitor_mod);
	swap(network_mod, source.network_mod);
	swap(ownModules, source.ownModules);
}

void circuit::ResetMonitors()
{
	mmz()->resetmonitor();
	totalCycles = continuedCycles = 0;
	monitorSamplesChanged.Trigger();
}

bool circuit::Simulate(int ncycles, bool resetBefore)
{
	if (resetBefore)
	{
		mmz()->resetmonitor();
		totalCycles = 0;
	}
	continuedCycles = 0;

	// Function to run the network
	bool ok = true;
	int n = ncycles;

	while (n > 0 && ok)
	{
		dmz()->executedevices(ok);
		if (ok)
		{
			n--;
			totalCycles++;
			continuedCycles++;
			mmz()->recordsignals();
		}
		else
		{
			cout << "Error: network is oscillating" << endl;
			ok = false;
		}
	}

	monitorSamplesChanged.Trigger();
	return ok;
}
