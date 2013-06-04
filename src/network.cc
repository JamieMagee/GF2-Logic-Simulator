#include <iostream>
#include "network.h"
 
using namespace std;

/***********************************************************************
 *
 * Returns list of devices.
 *
 */
devlink network::devicelist (void)
{
  return devs;
}


/***********************************************************************
 *
 * Returns link to device with specified name. Returns NULL if not       
 * found.                                                               
 *
 */
devlink network::finddevice (name id)
{
  devlink d;
  bool found;
  found = false;
  d = devs;
  while ((d != NULL) && (! found)) {
    found = (d->id == id);
    if (! found)
      d = d->next;
  }
  return d;
}


/***********************************************************************
 *
 * Returns link to input of device pointed to by dev with specified    
 * name.  Returns NULL if not found.                                    
 *
 */
inplink network::findinput (devlink dev, name id)
{
  inplink i;
  bool found;
  i = dev->ilist;
  found = false;
  while ((i != NULL) && (! found)) {
    found = (i->id == id);
    if (! found)
	i = i->next;
  }
  return i;
}


/***********************************************************************
 *
 * Returns link to output of device pointed to by dev with specified   
 * name.  Returns NULL if not found.                                    
 *
 */
outplink network::findoutput (devlink dev, name id)
{
  outplink o;
  bool found;
  o = dev->olist;
  found = false;
  while ((o != NULL) && (! found)) {
    found = (o->id == id);
    if (! found)
	o = o->next;
  }
  return o;
}


/***********************************************************************
 *
 * Adds a device to the device list with given name and returns a link 
 * to it via 'dev'.                                                    
 *
 */
void network::adddevice (devicekind dkind, name did, devlink& dev)
{
  dev = new devicerec;
  dev->id = did;
  dev->kind = dkind;
  dev->ilist = NULL;
  dev->olist = NULL;
  if (dkind != aclock) {        // device goes at head of list 
    if (lastdev == NULL)
	lastdev = dev;
    dev->next = devs;
    devs = dev;
  } else {                      // aclock devices must go last 
    dev->next = NULL;
    if (lastdev == NULL) {
      devs = dev;
      lastdev = dev;
    } else {
      lastdev->next = dev;
      lastdev = dev;
    }
  }
}


/***********************************************************************
 *
 * Adds an input to the device pointed to by 'dev' with the specified  
 * name.                                                               
 *
 */
void network::addinput (devlink dev, name iid)
{
  inplink i = new inputrec;
  i->id = iid;
  i->connect = NULL;
  i->next = dev->ilist;
  dev->ilist = i;
}


/***********************************************************************
 *
 * Adds an output to the device pointed to by 'dev' with the specified 
 * name.                                                               
 *
 */
void network::addoutput (devlink dev, name oid)
{
  outplink o = new outputrec;
  o->dev = dev;
  o->id = oid;
  o->sig = low;
  o->next = dev->olist;
  dev->olist = o;
}


/***********************************************************************
 *
 * Makes a connection between the 'inp' input of device 'idev' and the 
 * 'outp' output of device 'odev'. 'ok' is set true if operation       
 * succeeds.                                                           
 *
 */
void network::makeconnection (name idev, name inp, name odev, name outp, bool& ok)
{
  devlink din, dout;
  outplink o;
  inplink i;
  din = finddevice (idev);
  dout = finddevice (odev);
  ok = ((din != NULL) && (dout != NULL));
  if (ok) {
    o = findoutput (dout, outp);
    i = findinput (din, inp);
    ok = ((o != NULL) && (i != NULL));
    if (ok)
      i->connect = o;
  }
}


/***********************************************************************
 *
 * Checks that all inputs are connected to an output.   
 *
 */
void network::checknetwork (bool& ok, bool silent)
{
  devlink d;
  inplink i;
  ok = true;
  for (d = devs; d != NULL; d = d->next) 
    for (i = d->ilist; i != NULL; i = i->next)
      if (i->connect == NULL)
      {
        if (!silent)
        {
          cout << "Unconnected Input : ";
          nmz->writename (d->id);
          if (i->id != blankname) {
            cout << ".";
            nmz->writename (i->id);
          }
          cout << endl;
        }
        ok = false;
      }
    randomisedevices();
}


/***********************************************************************
 *
 * The constructor for the network module.
 * Remember the version of the names module that is used here and
 * shared with other modules.
 * Initialise the list of devices.
 *
 */
network::network (names* names_mod)
{
  nmz = names_mod;
  devs = NULL;
  lastdev = NULL;
}

// Delete a device, after disconnecting it. Monitors should be deleted too, but this function only handles deleting the devicerec and inputs+outputs (use circuit::RemoveDevice instead).
void network::deletedevice(devlink dTarget)
{
	if (!dTarget) return;
	devlink d = devicelist();
	devlink dPrev = NULL;
	while (d!=NULL)
	{
		if (d == dTarget)
		{
			outplink o = d->olist, oNext;
			while (o != NULL)
			{
				disconnectoutput(o);
				oNext = o->next;
				delete o;
				o = oNext;
			}
			inplink i = d->ilist, iNext;
			while (i != NULL)
			{
				iNext = i->next;
				delete i;
				i = iNext;
			}
			if (devs == d)
				devs = d->next;
			else
				dPrev->next = d->next;
			if (lastdev == d)
				lastdev = dPrev;
			break;
		}
		dPrev = d;
		d = d->next;
	}
}

// Disconnects all inputs connected to the given output
void network::disconnectoutput(outplink o)
{
	devlink d = devicelist();
	while (d!=NULL)
	{
		inplink i = d->ilist;
		while (i!=NULL)
		{
			if (i->connect == o)
				i->connect = NULL;
			i = i->next;
		}
		d = d->next;
	}
}

string network::getsignalstring(name dev, name p)
{
	if (dev==blankname)
		return "";
	string str = nmz->getnamestring(dev);
	if (p != blankname)
		str += "." + nmz->getnamestring(p);
	return str;
}

string network::getsignalstring(devlink d, outplink o)
{
	if (d==NULL)
		return "";
	name opn = blankname;
	if (o==NULL)
		return getsignalstring(d->id);
	return getsignalstring(d->id, o->id);
}

string network::getsignalstring(devlink d, inplink i)
{
	if (d==NULL)
		return "";
	name opn = blankname;
	if (i==NULL)
		return getsignalstring(d->id);
	return getsignalstring(d->id, i->id);
}

void network::randomisedevices()
{
	vector<devlink> devlist;
	vector<devlink> clklist;
	
	cout << "Sorted" << endl;
	for (devlink d = devs; d != NULL; d = d->next) 
	{
		cout << d->kind << endl;
		if (d->kind != aclock)
		{
			devlist.push_back(d);
		}
		else
		{
			clklist.push_back(d);
		}
	}
	std::random_shuffle(devlist.begin(), devlist.end());
	std::random_shuffle(clklist.begin(), clklist.end());
	cout << "Shuffled" << endl;
	devlink prevDevice = NULL;
	devs = NULL;
	for (std::vector<int>::size_type i = 0; i != devlist.size(); i++) 
	{
		if (prevDevice==NULL) devs = devlist[i];
		else prevDevice->next = devlist[i];
		prevDevice = devlist[i];
		cout << devlist[i]->kind << endl;
	}
	for (std::vector<int>::size_type i = 0; i != clklist.size(); i++) 
	{
		if (prevDevice==NULL) devs = clklist[i];
		else prevDevice->next = clklist[i];
		prevDevice = clklist[i];
		cout << clklist[i]->kind << endl;
	}
	if (prevDevice!=NULL) prevDevice->next = NULL;
	lastdev = prevDevice;
}
