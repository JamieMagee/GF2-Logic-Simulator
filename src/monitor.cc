#include <iostream>
#include "monitor.h"

using namespace std;

/***********************************************************************
 *
 * Sets a monitor on the 'outp' output of device 'dev' by placing an   
 * entry in the monitor table. 'ok' is set true if operation succeeds. 
 *
 */
void monitor::makemonitor (name dev, name outp, bool& ok)
{
	devlink d;
	outplink o;
	d = netz->finddevice(dev);
	ok = (d != NULL);
	if (ok)
	{
		o = netz->findoutput(d, outp);
		ok = (o != NULL);
		if (ok)
		{
			moninfo mon;
			mon.devid = dev;
			mon.op = o;
			mtab.push_back(mon);
		}
	}
}


/***********************************************************************
 *
 * Removes the monitor set on the 'outp' output of device 'dev'. 'ok' is
 * set true if operation succeeds.
 *
 */
void monitor::remmonitor (name dev, name outp, bool& ok)
{
	bool found;
    found = false;
	for (montable::iterator it=mtab.begin(); it!=mtab.end(); ++it)
	{
		if ((it->devid == dev) && (it->op->id == outp))
		{
			found = true;
			mtab.erase(it);
		}
	}
	ok = found;
}


/***********************************************************************
 *
 * Returns number of signals currently monitored.  
 *
 */
int monitor::moncount (void)
{
  return mtab.size();
}


/***********************************************************************
 *
 * Returns signal level of n'th monitor point. 
 *
 */
asignal monitor::getmonsignal (int n)
{
  return (mtab[n].op->sig);
}


/***********************************************************************
 *
 * Returns name of n'th monitor. 
 *
 */
void monitor::getmonname (int n, name& dev, name& outp)
{
  dev = mtab[n].devid;
  outp = mtab[n].op->id;
}


/***********************************************************************
 *
 * Initialises monitor memory in preparation for a new output sequence.
 *
 */
void monitor::resetmonitor (void)
{
  int n;
  for (n = 0; n < moncount (); n++)
    mtab[n].disp.clear();
  cycles = 0;
}


/***********************************************************************
 *
 * Called every clock cycle to record the state of each monitored     
 * signal.                                                            
 *
 */
void monitor::recordsignals (void)
{
  int n;
  for (n = 0; n < moncount (); n++)
    mtab[n].disp.push_back(getmonsignal(n));
  cycles++;
}

/***********************************************************************
 *
 * Access recorded signal trace, returns false if invalid monitor 
 * or cycle.
 *
 */
bool monitor::getsignaltrace(int m, int c, asignal &s)
{
  if ((c < cycles) && (m < moncount ()) && c<mtab[m].disp.size()) {
    s = mtab[m].disp[c];
    return true;
  }
  return false;
}

/***********************************************************************
 *
 * Get number of recorded cycles for the monitor, returns -1 if invalid
 * monitor.
 *
 */
int monitor::getsamplecount(int m)
{
  if (m < moncount ()) {
    return mtab[m].disp.size();
  }
  return -1;
}

/***********************************************************************
 *
 * Displays state of monitored signals.  
 *
 */
void monitor::displaysignals (void)
{
  const int margin = 20;
  int n, i;
  name dev, outp;
  int namesize;
  for (n = 0; n < moncount (); n++) {
    getmonname (n, dev, outp);
    namesize = nmz->namelength (dev);
    nmz->writename (dev);
    if (outp != blankname) {
      cout << ".";
      nmz->writename (outp);
      namesize = namesize + nmz->namelength (outp) + 1;
    }
    if ((margin - namesize) > 0) {
      for (i = 0; i < (margin - namesize - 1); i++)
        cout << " ";
      cout << ":";
    }
    for (i = 0; i < cycles; i++) 
      switch (mtab[n].disp[i]) {
        case high:    cout << "-"; break;
        case low:     cout << "_"; break;
        case rising:  cout << "/"; break;
        case falling: cout << "\\"; break;
      }
    cout << endl;
  }
}


/***********************************************************************
 *
 * Called to initialise the monitor module.  
 * Remember the names of the shared names and network modules.
 *
 */
monitor::monitor (names* names_mod, network* network_mod)
{
  nmz = names_mod;
  netz = network_mod;
}






