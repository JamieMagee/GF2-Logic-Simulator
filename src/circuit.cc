#include "circuit.h"
#include <algorithm>
using namespace std;

void CircuitElementInfoVector::push_back_all_devs(devlink d)
{
	while (d!=NULL)
	{
		push_back(CircuitElementInfo(d));
		d = d->next;
	}
}

void CircuitElementInfoVector::push_back_all_outputs(devlink d)
{
	while (d!=NULL)
	{
		push_back_dev_outputs(d);
		d = d->next;
	}
}

void CircuitElementInfoVector::push_back_all_inputs(devlink d)
{
	while (d!=NULL)
	{
		push_back_dev_inputs(d);
		d = d->next;
	}
}

void CircuitElementInfoVector::push_back_dev_outputs(devlink d)
{
	push_back_iolist(d, d->olist);
}

void CircuitElementInfoVector::push_back_dev_inputs(devlink d)
{
	push_back_iolist(d, d->ilist);
}

template <class T>
void CircuitElementInfoVector::push_back_iolist(devlink d, T item)
{
	while (item != NULL)
	{
		push_back(CircuitElementInfo(d, item));
		item = item->next;
	}
}

void CircuitElementInfoVector::UpdateSignalNames(circuit* c)
{
	for (CircuitElementInfoVector::iterator it=begin(); it<end(); it++)
	{
		if (!it->d)
		{
			it->namestr = "";
		}
		else
		{
			if (it->i)
				it->namestr = c->netz()->getsignalstring(it->d,it->i);
			else if (it->o)
				it->namestr = c->netz()->getsignalstring(it->d,it->o);
			else
				it->namestr = c->nmz()->getnamestring(it->d->id);
		}
	}
}



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

bool outputinfo_namestrcmp(const outputinfo a, const outputinfo b)
{
	return (a.namestr<b.namestr);
}

bool CircuitElementInfo_namestrcmp(const CircuitElementInfo a, const CircuitElementInfo b)
{
	return (a.namestr<b.namestr);
}

bool CircuitElementInfo_iconnect_namestrcmp(const CircuitElementInfo a, const CircuitElementInfo b)
{
	if (a.i && b.i && (a.i->connect!=NULL)!=(b.i->connect!=NULL))
		return (a.i->connect!=NULL) < (b.i->connect!=NULL);
	return (a.namestr<b.namestr);
}

bool circuit::GetUnmonitoredOutputs(vector<outputinfo> * unmonitoredOutputsRet)
{
	vector<outputinfo> unmonitoredOutputs;

	int monCount = mmz()->moncount();
	devlink d = netz()->devicelist();
	// Loop through devices
	while (d!=NULL)
	{
		// Loop through device outputs
		outplink o = d->olist;
		while (o!=NULL)
		{
			// Check whether this output is currently being monitored
			bool isMonitored = false;
			name monDev, monOut;
			for (int i=0; i<monCount; i++)
			{
				mmz()->getmonname(i, monDev, monOut);
				if (monDev==d->id && monOut==o->id)
				{
					isMonitored = true;
					break;
				}
			}
			if (!isMonitored)
			{
				outputinfo outinf;
				outinf.devname = d->id;
				outinf.outpname = o->id;
				outinf.namestr = netz()->getsignalstring(d->id, o->id);
				unmonitoredOutputs.push_back(outinf);
			}
			o = o->next;
		}
		d = d->next;
	}

	// If a pointer to a vector was supplied, put the list of unmonitored outputs in there
	if (unmonitoredOutputsRet)
		unmonitoredOutputsRet->swap(unmonitoredOutputs);

	// Return true if there are some unmonitored outputs in the circuit
	return (unmonitoredOutputs.size()>0);
}

bool circuit::IsDeviceNameValid(string devname)
{
	// Checks syntax of a device name string (but not whether a device already exists with that name or if it's a reserved word)
	if (!devname.length())
		return false;
	if (!isalpha(devname[0]))
		return false;
	for (string::iterator it=devname.begin(); it<devname.end(); ++it)
	{
		if (!isalpha(*it) && !isdigit(*it)) return false;
	}
	return true;
}

void circuit::RemoveDevice(devlink d)
{
	if (!d) return;

	// Remove monitors first
	outplink o = d->olist;
	bool ok;
	while (o != NULL)
	{
		mmz()->remmonitor(d->id, o->id, ok);
		o = o->next;
	}

	// Disconnect, release memory, and remove from linked list of devices
	netz()->deletedevice(d);

	circuitChanged.Trigger();
	monitorsChanged.Trigger();
}
