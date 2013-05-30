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
  netz = new network(nmz);
  dmz = new devices(nmz, netz);
  mmz = new monitor(nmz, netz);

#ifdef USE_GUI
  // glutInit cannot cope with Unicode command line arguments, so we pass
  // it some fake ASCII ones instead
  char **tmp1; int tmp2 = 0; glutInit(&tmp2, tmp1);
  // Construct the GUI
  MyFrame *frame = new MyFrame(NULL, wxT("Logic simulator"), wxDefaultPosition,  wxSize(800, 600), nmz, dmz, mmz, netz);
  frame->Show(true);
  if (argc == 2)
  {
    frame->loadFile(wxString(argv[1]).mb_str());
  }
  return(true); // enter the GUI event loop
#else
  smz = new scanner(nmz, wxString(argv[1]).mb_str(), ok);
  if (!ok)
  {
	return(false);	
  }
  erz = new error(smz);
  pmz = new parser(netz, dmz, mmz, smz, erz);
  if (pmz->readin ()) { // check the logic file parsed correctly
    // Construct the text-based interface
    userint umz(nmz, dmz, mmz);
    umz.userinterface();
  }
  return(false); // exit the application
#endif /* USE_GUI */
}
