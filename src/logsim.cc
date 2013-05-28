#include "logsim.h"
#include "userint.h"
#include "gui.h"
#include <GL/glut.h>

#define USE_GUI

IMPLEMENT_APP(MyApp)
  
bool MyApp::OnInit()
  // This function is automatically called when the application starts
{
#ifndef USE_GUI
  if (argc != 2) { // check we have one command line argument
    wcout << "Usage:      " << argv[0] << " [filename]" << endl;
    exit(1);
  }
#endif

  // Construct the six classes required by the innards of the logic simulator
  nmz = new names();
  erz = new error();
  netz = new network(nmz);
  dmz = new devices(nmz, netz);
  mmz = new monitor(nmz, netz);
  smz = new scanner(nmz, wxString(argv[1]).mb_str());
  pmz = new parser(netz, dmz, mmz, smz, erz);
  

	//TODO: remove this when the scanner and parser work
	if (argc == 2 && wxString(argv[1]) == wxT("builtin-example"))
	{
		// A built-in example circuit, to give something more interesting to test the GUI with
		// XOR gate built from NANDS, with a switch&clock and a clock as the inputs (S1 ANDed with C2, and C1)
		name c1=nmz->lookup("C1"), c2=nmz->lookup("C2"), s1=nmz->lookup("S1"), g1=nmz->lookup("G1"), g2=nmz->lookup("G2"), g3=nmz->lookup("G3"), g4=nmz->lookup("G4"), gi1 = nmz->lookup("GI1");
		name i1=nmz->lookup("I1"), i2=nmz->lookup("I2");
		name dt1=nmz->lookup("DT1");
		name idata=nmz->lookup("DATA"), iclk=nmz->lookup("CLK"), iset=nmz->lookup("SET"), iclear=nmz->lookup("CLEAR");
		bool ok;
		dmz->makedevice(nandgate, g1, 2, ok);
		dmz->makedevice(nandgate, g2, 2, ok);
		dmz->makedevice(nandgate, g3, 2, ok);
		dmz->makedevice(nandgate, g4, 2, ok);
		dmz->makedevice(andgate, gi1, 2, ok);
		dmz->makedevice(aclock, c1, 3, ok);
		dmz->makedevice(aclock, c2, 5, ok);
		dmz->makedevice(aswitch, s1, 1, ok);

		netz->makeconnection(gi1, i1, s1, blankname, ok);
		netz->makeconnection(gi1, i2, c2, blankname, ok);
		netz->makeconnection(g1, i1, gi1, blankname, ok);
		netz->makeconnection(g1, i2, c1, blankname, ok);
		netz->makeconnection(g2, i1, gi1, blankname, ok);
		netz->makeconnection(g2, i2, g1, blankname, ok);
		netz->makeconnection(g3, i1, g1, blankname, ok);
		netz->makeconnection(g3, i2, c1, blankname, ok);
		netz->makeconnection(g4, i1, g2, blankname, ok);
		netz->makeconnection(g4, i2, g3, blankname, ok);

		mmz->makemonitor(g4, blankname, ok);
		mmz->makemonitor(c1, blankname, ok);
		mmz->makemonitor(s1, blankname, ok);

		dmz->makedevice(dtype, dt1, 0, ok);
		netz->makeconnection(dt1, idata, g1, blankname, ok);
		netz->makeconnection(dt1, iclk, c1, blankname, ok);
		netz->makeconnection(dt1, iset, g4, blankname, ok);
		netz->makeconnection(dt1, iclear, c2, blankname, ok);
		mmz->makemonitor(dt1, nmz->lookup("Q"), ok);
		mmz->makemonitor(dt1, nmz->lookup("QBAR"), ok);

		netz->checknetwork(ok);
		if (!ok)
		{
			cout << "Error: something went wrong with loading the example circuit" << endl;
			exit(1);
		}
	}

#ifdef USE_GUI
  // glutInit cannot cope with Unicode command line arguments, so we pass
  // it some fake ASCII ones instead
  char **tmp1; int tmp2 = 0; glutInit(&tmp2, tmp1);
  // Construct the GUI
  MyFrame *frame = new MyFrame(NULL, wxT("Logic simulator"), wxDefaultPosition,  wxSize(800, 600), nmz, dmz, mmz, netz);
  frame->Show(true);
  if (argc == 2 && wxString(argv[1]) != wxT("builtin-example"))
  {
    frame->loadFile(wxString(argv[1]).mb_str());
  }
  return(true); // enter the GUI event loop
#else
  if (pmz->readin ()) { // check the logic file parsed correctly
    // Construct the text-based interface
    userint umz(nmz, dmz, mmz);
    umz.userinterface();
  }
  return(false); // exit the application
#endif /* USE_GUI */
}
