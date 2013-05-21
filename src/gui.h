#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include "names.h"
#include "devices.h"
#include "monitor.h"
#include "network.h"

enum { 
  MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  MY_TEXTCTRL_ID,
  MY_BUTTON_ID,
  OUTPUT_TEXTCTRL_ID,
  SIMCTRL_BUTTON_RUN_ID,
  SIMCTRL_BUTTON_CONT_ID
}; // widget identifiers

class MyGLCanvas;
class SimCtrls;

class MyFrame: public wxFrame
{
 public:
  MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size, 
	  names *names_mod = NULL, devices *devices_mod = NULL, monitor *monitor_mod = NULL, network *net_mod = NULL,
	  long style = wxDEFAULT_FRAME_STYLE); // constructor
  virtual ~MyFrame();
  bool loadFile(const char * filename);// loads the given file, returns true if successful
 private:
  MyGLCanvas *canvas;                     // GL drawing area widget to draw traces
  wxSpinCtrl *spin;                       // control widget to select the number of cycles
  wxTextCtrl *outputTextCtrl;             // textbox to display messages sent to cout (e.g. error messages from scanner and parser)
  wxStreamToTextRedirector *outputTextRedirect;
  names *nmz;                             // pointer to names class
  devices *dmz;                           // pointer to devices class
  monitor *mmz;                           // pointer to monitor class
  network *netz;                          // pointer to network class
  int continuedCycles;// how many simulation cycles were completed last time the run or continue button was used
  int totalCycles;// how many simulation cycles have been completed

  void clearCircuit();// clear all devices, connections, and monitors

  void runnetwork(int ncycles);           // function to run the logic network
  void OnExit(wxCommandEvent& event);     // callback for exit menu item
  void OnAbout(wxCommandEvent& event);    // callback for about menu item
  void OnOpenFile(wxCommandEvent& event); // callback for open file menu item
  void OnButtonRun(wxCommandEvent& event);
  void OnButtonContinue(wxCommandEvent& event);
  void OnSpin(wxSpinEvent& event);        // callback for spin control
  void OnText(wxCommandEvent& event);     // callback for text entry field
  DECLARE_EVENT_TABLE()
};
    
class MyGLCanvas: public wxGLCanvas
{
 public:
  MyGLCanvas(wxWindow *parent, wxWindowID id = wxID_ANY, monitor* monitor_mod = NULL, names* names_mod = NULL,
	     const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
	     const wxString& name = wxT("MyGLCanvas")); // constructor
  void Render(wxString text=wxT("")); // function to draw canvas contents
  void SimulationRun(int totalCycles_new, int continuedCycles_new);
 private:
  bool init;                         // has the GL context been initialised?
  int continuedCycles;// how many simulation cycles were completed last time the run or continue button was used
  int totalCycles;// how many simulation cycles have been completed
  monitor *mmz;                      // pointer to monitor class, used to extract signal traces
  names *nmz;                        // pointer to names class, used to extract signal names
  void InitGL();                     // function to initialise GL context
  void OnSize(wxSizeEvent& event);   // callback for when canvas is resized
  void OnPaint(wxPaintEvent& event); // callback for when canvas is exposed
  void OnMouse(wxMouseEvent& event); // callback for mouse events inside canvas
  void DrawSignalTrace(int xOffset, int yOffset, float xScale, int height, int padding, int mon, int cycles);
  void DrawText(int x, int y, wxString txt, void *font=NULL);
  int GetTextWidth(wxString txt, void *font=NULL);
  DECLARE_EVENT_TABLE()
};

#endif /* gui_h */
