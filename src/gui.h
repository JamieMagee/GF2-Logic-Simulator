#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/scrolwin.h>
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

int GetGlutTextWidth(wxString txt, void *font=NULL);
void DrawGlutText(int x, int y, wxString txt, void *font=NULL);

class MyGLCanvas;
class SimCtrls;

void wxRect_GlVertex(const wxRect& r);

class GLCanvasMonitorTrace
{
	friend class MyGLCanvas;
public:
	GLCanvasMonitorTrace();
	GLCanvasMonitorTrace(int newMonId, monitor *monitor_mod, names *names_mod);
	void SetModules(monitor *monitor_mod, names *names_mod);
	// Get or set the monitor id (the "n" in "n'th monitor" in monitor class calls, also determines position)
	int GetMonitorId();
	void SetMonitorId(int newMonId);
	// Notify of a change in the number of displayed cycles
	void SimulationRun(int totalCycles_new, int continuedCycles_new);
	// Draw the signal trace, visible coordinates are used so that time is not wasted in drawing areas hidden due to scrolling.
	void Draw(MyGLCanvas *canvas, const wxRect& visibleRegion);
	void DrawName(MyGLCanvas *canvas, const wxRect& visibleRegion);
	// Get the width in pixels of the name, used by MyGLCanvas.Render() to determine how much space to leave between the traces and the edge of the canvas
	int GetNameWidth();
	// Set the geometry and positioning of the monitor trace. xOffset and yOffset are the top left corner of the bounding box of the first trace. xScale is the x-axis scale (the number of pixels per cycle). height is the height of the signal trace itself, padding the distance between the top of the signal trace and the edge of the graph background, and spacing the vertical distance between centre lines of consecutive monitors.
	void SetGeometry(int xOffset_new, int yOffset_new, double xScale_new, int sigHeight_new, int padding_new, int spacing_new, int xBgName_new, int axisLabelInterval_new);
private:
	int monId;
	monitor *mmz; // pointer to monitor class, used to extract signal traces
	names *nmz; // pointer to names class, used to extract signal names
	wxString monName;
	int monNameWidth;
	bool geometrySet;
	int xOffset, yCentre, sigHeight, padding, spacing, xBgName;
	int axisLabelInterval;// cycles between numbers on x axis
	double xScale;
	wxRect backgroundRegion;
	void UpdateName();// Update monName and monNameWidth
	int continuedCycles;// how many simulation cycles were completed last time the run or continue button was used
	int totalCycles;// how many simulation cycles have been completed
};

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
    
class MyGLCanvas: public wxGLCanvas, public wxScrollHelperNative
{
 public:
  MyGLCanvas(wxWindow *parent, wxWindowID id = wxID_ANY, monitor* monitor_mod = NULL, names* names_mod = NULL,
	     const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0,
	     const wxString& name = wxT("MyGLCanvas")); // constructor
  void Render(wxString text=wxT("")); // function to draw canvas contents
  void SimulationRun(int totalCycles_new, int continuedCycles_new);
  void MonitorsChanged();
  void UpdateMinCanvasSize();
 private:
  bool init;                         // has the GL context been initialised?
  int continuedCycles;// how many simulation cycles were completed last time the run or continue button was used
  int totalCycles;// how many simulation cycles have been completed
  monitor *mmz;                      // pointer to monitor class, used to extract signal traces
  names *nmz;                        // pointer to names class, used to extract signal names
  int maxMonNameWidth;
  vector<GLCanvasMonitorTrace> mons;
  void InitGL();                     // function to initialise GL context
  void OnSize(wxSizeEvent& event);   // callback for when canvas is resized
  void OnPaint(wxPaintEvent& event); // callback for when canvas is exposed
  void OnMouse(wxMouseEvent& event); // callback for mouse events inside canvas
  int scrollX, scrollY;
  int minXScale, maxXScale;
public:
  virtual void ScrollWindow(int dx, int dy, const wxRect* rect = (wxRect *)NULL);

  // copied from wxScrolledWindow
#ifdef __WXMSW__
    virtual WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif // __WXMSW__
  WX_FORWARD_TO_SCROLL_HELPER()
  // end copied from wxScrolledWindow

private:
  DECLARE_EVENT_TABLE()
};

#endif /* gui_h */
