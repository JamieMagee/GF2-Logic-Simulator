#include "devices.h"
#include "monitor.h"


using namespace std;

/***********************************************************************
 *
 * Used to print out signal values for debugging in showdevice.
 *
 */
void devices::outsig (asignal s)
{
  switch (s) {
    case high:    cout << "high";    break;
    case low:     cout << "low";     break;
    case rising:  cout << "rising";  break;
    case falling: cout << "falling"; break;
  }
}


/***********************************************************************
 *
 * Used to print out device details and signal values 
 * for debugging in executedevices.
 *
 */
void devices::showdevice (devlink d)
{
  inplink  i;
  outplink o;
  cout << "   Device: ";
  nmz->writename (d->id);
  cout << "  Kind: ";
  writedevice (d->kind);
  cout << endl;
  cout << "   Inputs:" << endl;
  for (i = d->ilist; i != NULL; i = i->next) {
    cout << "      ";
    nmz->writename (i->id);
    cout << " ";
    outsig (i->connect->sig);
    cout << endl;
  }
  cout << "   Outputs:";
  for (o = d->olist; o != NULL; o = o->next) {
    cout << "      ";
    nmz->writename (o->id);
    cout << " ";
    outsig (o->sig);
    cout << endl;
  }
  cout << endl;
}


/***********************************************************************
 *
 * Sets the state of the named switch. 'ok' returns false if switch  
 * not found.                                                        
 *
 */
void devices::setswitch (name sid, asignal level, bool& ok)
{
  devlink d;
  d = netz->finddevice (sid);
  ok = (d != NULL);
  if (ok) {
    ok = (d->kind == aswitch);
    if (ok)
      d->swstate = level;
  }
}


/***********************************************************************
 *
 * Used to make new switch devices.
 * Called by makedevice.
 *
 */
void devices::makeswitch (name id, int setting, bool& ok)
{
  devlink d;
  ok = (setting <= 1);
  if (ok) {
    netz->adddevice (aswitch, id, d);
    netz->addoutput (d, blankname);
    d->swstate = (setting == 0) ? low : high;
  }
}


/***********************************************************************
 *
 * Used to make new clock devices.
 * Called by makedevice.
 *
 */
void devices::makeclock (name id, int frequency)
{
  devlink d;
  netz->adddevice (aclock, id, d);
  netz->addoutput (d, blankname);
  d->frequency = frequency;
  d->counter = 0;
}


/***********************************************************************
 *
 * Used to make new AND, NAND, OR, NOR and XOR gates. 
 * Called by makedevice.
 *
 */
void devices::makegate (devicekind dkind, name did, int ninputs, bool& ok)
{
  const int maxinputs = 16;
  devlink d;
  int n;
  namestring iname;
  ok = (ninputs <= maxinputs);
  if (ok) {
    netz->adddevice (dkind, did, d);
    netz->addoutput (d, blankname);
    for (n = 1; n <= ninputs; n++) {
      netz->addinput(d, GetGateInputName(n));
    }
  }
}


/***********************************************************************
 *
 * Used to make new D-type bistable devices.
 * Inputs: D, clock, preset and clear.
 * Outputs: Q, QBAR.
 * Called by makedevice.
 *
 */
void devices::makedtype (name id)
{
  devlink d;
  netz->adddevice (dtype, id, d);
  netz->addinput (d, datapin);
  netz->addinput (d, clkpin);
  netz->addinput (d, setpin);
  netz->addinput (d, clrpin);
  netz->addoutput (d, qpin);
  netz->addoutput (d, qbarpin);
  d->memory = low;
  d->holdCountdown = 0;
  d->steadyCounter = 0;
}

/***********************************************************************
 *
 * Used to make new clock devices.
 * Called directly by GUI and Parser
 *
 */
void devices::makesiggen (name id, sequence waveform)
{
	devlink d;
	netz->adddevice (siggen, id, d);
	netz->addoutput (d, blankname);
	d->waveform = waveform;
	d->counter = 0;
}

/***********************************************************************
 *
 * Adds a device to the network of the specified kind and name.  The  
 * variant is used with such things as gates where it specifies the   
 * number of inputs. 'ok' returns true if operation succeeds.         
 *
 */
void devices::makedevice (devicekind dkind, name did, int variant, bool& ok)
{
  ok = true;
  switch (dkind) {
    case aswitch:
      makeswitch (did, variant, ok);
      break;
    case aclock:
      makeclock (did, variant);
      break;
    case andgate:
    case nandgate:
    case orgate:
    case norgate:
      makegate (dkind, did, variant, ok);
      break;
    case xorgate:
      makegate (dkind, did, 2, ok);
      break;
    case dtype:
      makedtype(did);
      break;
    default:
		ok = false;
  }
}


/***********************************************************************
 *
 * Update output `o' in the direction of signal `target'.
 * Set steadystate to false if this results in a change in o->sig.
 *
 */
void devices::signalupdate (asignal target, outplink o)
{
  switch (o->sig) {
    case falling:
    case low:
      o->nextsig = (target == high) ? rising : low;
      break;
    case rising:
    case high:
      o->nextsig = (target == low) ? falling : high;
      break;
  }
  if (o->nextsig != o->sig)
    steadystate = false;
}


/***********************************************************************
 *
 * Returns the inverse of a signal.
 *
 */
asignal devices::inv (asignal s)
{
  return ((s == high) ? low : high);
}


/***********************************************************************
 *
 * Used to simulate the operation of switch devices.
 * Called by executedevices.
 *
 */
void devices::execswitch (devlink d)
{
  signalupdate (d->swstate, d->olist);
}


/***********************************************************************
 *
 * Used to simulate the operation of AND, OR, NAND and NOR gates.
 * Called by executedevices.
 * Meaning of arguments: gate output is 'y' iff all inputs are 'x'
 *
 */
void devices::execgate (devlink d, asignal x, asignal y)
{
  asignal newoutp;
  inplink inp = d->ilist;
  outplink outp = d->olist;
  newoutp = y;
  while ((inp != NULL) && (newoutp == y)) {
    if (inp->connect->sig == inv (x))
      newoutp = inv (y);
    inp = inp->next;
  }
  signalupdate (newoutp, outp);
}


/***********************************************************************
 *
 * Used to simulate the operation of exclusive or gates.
 * Called by executedevices.
 *
 */
void devices::execxorgate(devlink d)
{
  asignal newoutp;
  if (d->ilist->connect->sig == d->ilist->next->connect->sig)
    newoutp = low;
  else
    newoutp = high;
  signalupdate (newoutp, d->olist);
}


/***********************************************************************
 *
 * Used to simulate the operation of D-type bistables.
 * Called by executedevices. The signal on the data input
 * immediately BEFORE the clock edge is transferred to the
 * Q output. We are effectively assuming a negligible but
 * nonzero setup time, and a zero hold time.
 *
 */
void devices::execdtype (devlink d, int cycles)
{
	asignal datainput, clkinput, setinput, clrinput;
	inplink i;
	outplink qout, qbarout;
	i = netz->findinput (d, datapin); datainput = i->connect->sig;
	i = netz->findinput (d, clkpin);  clkinput  = i->connect->sig;
	i = netz->findinput (d, clrpin);  clrinput  = i->connect->sig;
	i = netz->findinput (d, setpin);  setinput  = i->connect->sig;
	qout = netz->findoutput (d, qpin);
	qbarout = netz->findoutput (d, qbarpin);
	if (datainput==rising || datainput == falling)
		d->steadyCounter = 0;
	else if (d->steadyCounter<3)
		d->steadyCounter++;
	if (d->holdCountdown>0)
	{
		if (datainput==rising || datainput == falling)
		{
			d->memory = (rand()%2 ? high : low);
			if (debuggingIndeterminate)
			{
				cout << "Warning: indeterminate output for D-type " << nmz->getnamestring(d->id) << ", input changed during hold time" << endl;
			}
			d->holdCountdown = 0;
		}
		else
		{
			d->holdCountdown--;
			if (d->holdCountdown==0)
			{
				d->memory = datainput;
			}
			else
			{
				steadystate = false;
			}
		}
	}
	if (clkinput == rising)
	{
		if (d->steadyCounter>=2)
		{
			d->holdCountdown = 1;
		}
		else
		{
			d->memory = (rand()%2 ? high : low);
			d->holdCountdown = 0;
			if (debuggingIndeterminate)
			{
				if (d->steadyCounter==0)
				{
					cout << "Warning: indeterminate output for D-type " << nmz->getnamestring(d->id) << ", input changed during clock rise" << endl;
				}
				else
				{
					cout << "Warning: indeterminate output for D-type " << nmz->getnamestring(d->id) << ", input changed during setup time" << endl;
				}
			}
		}
	}
	if (setinput == high)
		d->memory = high;
	if (clrinput == high)
		d->memory = low;
	signalupdate (d->memory, qout);
	signalupdate (inv (d->memory), qbarout);
}


/***********************************************************************
 *
 * Used to simulate the operation of clock devices.
 * Called by executedevices.
 *
 */
void devices::execclock(devlink d)
{
  signalupdate(d->memory, d->olist);
}

/***********************************************************************
 *
 * Used to simulate the operation of siggen devices.
 * Called by executedevices.
 *
 */
void devices::execsiggen(devlink d)
{
	if (d->waveform[d->counter])
	{
		signalupdate (high, d->olist);
	}
	else
	{
		signalupdate (low, d->olist);
	}
}


/***********************************************************************
 *
 * Increment the counters in the clock devices and initiate changes
 * in their outputs when the end of their period is reached.
 * Called by executedevices.
 *
 */
void devices::updateclocks (void)
{
  devlink d;
  for (d = netz->devicelist (); d != NULL; d = d->next) {
    if (d->kind == aclock) {
      if (d->counter >= d->frequency) {
		d->counter = 0;
		if (d->olist->sig == high)
		{
		  d->memory = low;
		}
		else
		{
		  d->memory = high;
		}
      }
      (d->counter)++;
    }
  }
}

/***********************************************************************
 *
 * Increment the counters in the clock devices and initiate changes
 * in their outputs when the end of their period is reached.
 * Called by executedevices.
 *
 */
void devices::updatesiggens (void)
{
	devlink d;
	for (d = netz->devicelist (); d != NULL; d = d->next) {
		if (d->kind == siggen)
		{
			(d->counter) ++;
			if (d->counter >= d->waveform.size())
			{
				d->counter=0;
			}
		}
	}
}

/***********************************************************************
 *
 * Executes all devices in the network to simulate one complete clock 
 * cycle. 'ok' is returned false if network fails to stabilise (i.e.  
 * it is oscillating).                                            
 *
 */
void devices::executedevices (bool& ok, monitor* mmz)
{
  int maxmachinecycles = 20;
  int count = 0;
  devlink d;
  int machinecycle;
  if (debugging)
    cout << "Start of execution cycle" << endl;
  updateclocks ();
  machinecycle = 0;
  do {
    machinecycle++;
    if (debugging)
      cout << "machine cycle # " << machinecycle << endl;
    steadystate = true;
    for (d = netz->devicelist (); d != NULL; d = d->next) {
      switch (d->kind) {
        case aswitch:  execswitch (d);           break;
        case aclock:   execclock (d);            break;
        case orgate:   execgate (d, low, low);   break;
        case norgate:  execgate (d, low, high);  break;
        case andgate:  execgate (d, high, high); break;
        case nandgate: execgate (d, high, low);  break;
        case xorgate:  execxorgate (d);          break;
        case dtype:    execdtype (d, machinecycle);break;   
        case siggen:   execsiggen (d);           break;
        case baddevice:				 break;
      }
      if (machinecycle==1) count++;
      if (debugging)
        showdevice (d);
    }
    for (d = netz->devicelist (); d != NULL; d = d->next) {
		for (outplink o = d->olist; o != NULL; o = o->next) {
			o->sig = o->nextsig;
		}
	}
    if (mmz)
        mmz->recordsignals(true);
    if (machinecycle==1) maxmachinecycles = 20 + 2*count;
  } while ((! steadystate) && (machinecycle < maxmachinecycles));
  if (debugging)
    cout << "End of execution cycle" << endl;
  updatesiggens();
  ok = steadystate;
}


/***********************************************************************
 *
 * Prints out the given device kind. 
 * Used by showdevice.
 *
 */
void devices::writedevice (devicekind k)
{
  nmz->writename (dtab[k]);
}


/***********************************************************************
 *
 * Returns the kind of device corresponding to the given name.   
 * 'baddevice' is returned if the name is not a legal device.    
 *
 */
devicekind devices::devkind (name id)
{
  devicekind d;
  d = aswitch;
  while ((d != baddevice) && (dtab[d] != id))
    d = static_cast<devicekind>(d + 1);
  return (d);
}


/***********************************************************************
 *
 * Set the state of the internal debugging flag.
 *
 */
void devices::debug (bool on)
{
  debugging = on;
}
void devices::debugIndeterminate (bool on)
{
  debuggingIndeterminate = on;
}


/***********************************************************************
 * 
 * Constructor for the devices class.
 * Registers the names of all the possible devices.
 * 
 */
devices::devices (names* names_mod, network* net_mod)
{
  nmz = names_mod;
  netz = net_mod;
  dtab[aswitch]   =  nmz->lookup("SWITCH");
  dtab[aclock]    =  nmz->lookup("CLOCK");
  dtab[andgate]   =  nmz->lookup("AND");
  dtab[nandgate]  =  nmz->lookup("NAND");
  dtab[orgate]    =  nmz->lookup("OR");
  dtab[norgate]   =  nmz->lookup("NOR");
  dtab[xorgate]   =  nmz->lookup("XOR");
  dtab[dtype]     =  nmz->lookup("DTYPE");
  dtab[siggen]	  =  nmz->lookup("SIGGEN");
  dtab[baddevice] =  blankname;
  debugging = false;
  debuggingIndeterminate = false;
  datapin = nmz->lookup("DATA");
  clkpin  = nmz->lookup("CLK");
  setpin  = nmz->lookup("SET");
  clrpin  = nmz->lookup("CLEAR");
  qpin    = nmz->lookup("Q");
  qbarpin = nmz->lookup("QBAR");
}

name devices::GetGateInputName(int n)
{
	// moved from makegate() into new function
	namestring iname = "I";
	if (n < 10) {
		iname += ((char) n) + '0';
	} else {
		iname += ((char) (n / 10)) + '0';
		iname += ((char) (n % 10)) + '0';
	}
	return nmz->lookup(iname);
}

void devices::SetGateInputCount(devlink d, int newCount)
{
	if (!d) return;
	int oldCount = GetLinkedListLength(d->ilist);
	if (newCount<oldCount)
	{
		for (int i=0; i<oldCount-newCount; i++)
		{
			inplink nextI = d->ilist->next;
			delete d->ilist;
			d->ilist = nextI;
		}
	}
	else
	{
		for (int i=oldCount+1; i<=newCount; i++)
		{
			netz->addinput(d, GetGateInputName(i));
		}
	}
}

bool devices::CheckDeviceInputs(devlink d)
{
	inplink i = d->ilist;
	while (i != NULL)
	{
		if (i->connect == NULL) return false;
		i = i->next;
	}
	return true;
}
