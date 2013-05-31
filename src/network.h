#ifndef network_h
#define network_h

#include "names.h"
#include <string>

using namespace std;

/* Network specification */

typedef enum {falling, low, rising, high} asignal;
typedef enum {aswitch, aclock, andgate, nandgate, orgate,
	      norgate, xorgate, dtype, siggen, baddevice} devicekind;

struct devicerec;
struct outputrec {
  name       id;
  asignal    sig;
  outputrec* next;
  devicerec* dev;
};
typedef outputrec* outplink;
struct inputrec {
  name      id;
  outplink  connect;
  inputrec* next;
};
typedef inputrec* inplink;
struct devicerec {
  name id;
  inplink ilist;
  outplink olist;
  devicerec* next;
  devicekind kind;
  /* the next elements are only used by some of the device kinds */
  asignal swstate;      // used when kind == aswitch
  int frequency;        // used when kind == aclock
  int counter;          // used when kind == aclock
  asignal memory;       // used when kind == dtype
  int signal;			// used when kind == siggen
};
typedef devicerec* devlink;

template <class T>
int GetLinkedListLength(T item)
{
	int count = 0;
	while (item != NULL)
	{
		count++;
		item = item->next;
	}
	return count;
}

class network {
  names* nmz;  // the instatiation of the names class that we are going to use.

 public:
  devlink devicelist (void);
    /* Returns list of devices                                             */
 
  devlink finddevice (name id);
   /* Returns link to device with specified name. Returns NULL if not       */
   /* found.                                                               */
 
  inplink findinput (devlink dev, name id);
    /* Returns link to input of device pointed to by dev with specified    */
    /* name.  Returns NULL if not found.                                    */
 
  outplink findoutput (devlink dev, name id);
    /* Returns link to output of device pointed to by dev with specified   */
    /* name.  Returns NULL if not found.                                    */
 
  void adddevice (devicekind dkind, name did, devlink& dev);
    /* Adds a device to the device list with given name and returns a link */
    /* to it via 'dev'.                                                    */
 
  void addinput (devlink dev, name iid);
    /* Adds an input to the device pointed to by 'dev' with the specified  */
    /* name.                                                               */
 
  void addoutput (devlink dev, name oid);
    /* Adds an output to the device pointed to by 'dev' with the specified */
    /* name.                                                               */

  void makeconnection (name idev, name inp, name odev, name outp, bool& ok);
    /* Makes a connection between the 'inp' input of device 'idev' and the */
    /* 'outp' output of device 'odev'. 'ok' is set true if operation       */
    /* succeeds.                                                           */
 
  void checknetwork (bool& ok, bool silent=false);
    /* Checks that all inputs are connected to an output.                  */
 
  network (names* names_mod);
  /* Called on system initialisation.                                      */

  string getsignalstring(name dev, name p=blankname);
  string getsignalstring(devlink dev, outplink o);
  string getsignalstring(devlink dev, inplink i);
  /* Returns the string corresponding to the given device and pin          */

  // Disconnects all inputs connected to the given output
  void disconnectoutput(outplink o);
  
  // Deletes a device (after disconnecting the outputs)
  void deletedevice(devlink dTarget);

 private:
  devlink devs;          // the list of devices
  devlink lastdev;       // last device in list of devices

};

#endif /* network_h */





