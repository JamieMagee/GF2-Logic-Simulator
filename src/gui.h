#ifndef gui_h
#define gui_h

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/checklst.h>
#include <wx/scrolwin.h>
#include <wx/panel.h>
#include "names.h"
#include "devices.h"
#include "monitor.h"
#include "network.h"
#include "circuit.h"
#include <vector>
#include <string>

using namespace std;

enum { 
  MY_SPINCNTRL_ID = wxID_HIGHEST + 1,
  MY_TEXTCTRL_ID,
  MY_BUTTON_ID,
  OUTPUT_TEXTCTRL_ID,
  SIMCTRL_BUTTON_RUN_ID,
  SIMCTRL_BUTTON_CONT_ID,
  MONITORS_ADD_BUTTON_ID,
  MONITORS_DEL_BUTTON_ID,
  SWITCHES_CTRL_ID
}; // widget identifiers

int GetGlutTextWidth(wxString txt, void *font=NULL);
void DrawGlutText(int x, int y, wxString txt, void *font=NULL);

class MyGLCanvas;
class SwitchesCheckListBox;

void wxRect_GlVertex(const wxRect& r);

class GLCanvasMonitorTrace
{
public:
	GLCanvasMonitorTrace();
	GLCanvasMonitorTrace(int newMonId, circuit* c);
	// Get or set the monitor id (the "n" in "n'th monitor" in monitor class calls, also determines position)
	int GetMonitorId();
	void SetMonitorId(int newMonId);
	// Notify of a change in the number of displayed cycles
	void OnMonitorSamplesChanged(int totalCycles_new, int continuedCycles_new);
	// Draw the signal trace, visible coordinates are used so that time is not wasted in drawing areas hidden due to scrolling.
	void Draw(MyGLCanvas *canvas, const wxRect& visibleRegion);
	void DrawName(MyGLCanvas *canvas, const wxRect& visibleRegion);
	// Get the width in pixels of the name, used by MyGLCanvas.Render() to determine how much space to leave between the traces and the edge of the canvas
	int GetNameWidth();
	// Set the geometry and positioning of the monitor trace. 
	// xOffset - the x position of the graph origin
	// yCentre - the y position of the centre of the drawn signal (halfway between high and low signal levels)
	// xScale - the x-axis scale (the number of pixels per cycle). 
	// height - the height of the signal trace line itself, 
	// padding - the distance between the top of the signal trace and the edge of the graph background
	// spacing - the vertical distance between centre lines of consecutive monitor traces. 
	// xBgName - the distance between the edge of the screen and the faint signal name drawn on top of the traces when the text to the left is invisible.
	void SetGeometry(int xOffset_new, int yCentre_new, double xScale_new, int sigHeight_new, int padding_new, int spacing_new, int xBgName_new, int axisLabelInterval_new);
private:
	int monId;
	circuit* c;
	wxString monName;// cached monitor name, to avoid constructing it again every time the signal is drawn
	int monNameWidth;// width in pixels of monitor name
	bool geometrySet;// whether SetGeometry() has been called

	// variables controlling position and size of the trace, see SetGeometry() for details
	int xOffset, yCentre, sigHeight, padding, spacing, xBgName;
	int axisLabelInterval;// cycles between numbers on x axis
	double xScale;

	void UpdateName();// Update monName and monNameWidth
	int continuedCycles;// how many simulation cycles were completed last time the run or continue button was used
	int totalCycles;// how many simulation cycles have been completed in total
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
  SwitchesCheckListBox *switchesCtrl;
	wxPanel *simctrls_container;
	wxButton *simctrl_run;
	wxButton *simctrl_continue;
	wxButton *monitors_add_btn;
	wxButton *monitors_rem_btn;
	circuit* c;

  void OnExit(wxCommandEvent& event);     // callback for exit menu item
  void OnAbout(wxCommandEvent& event);    // callback for about menu item
  void OnOpenFile(wxCommandEvent& event); // callback for open file menu item
  void OnButtonRun(wxCommandEvent& event);
  void OnButtonContinue(wxCommandEvent& event);
	void OnButtonAddMon(wxCommandEvent& event);
	void OnButtonDelMon(wxCommandEvent& event);
	void UpdateControlStates();
 
  DECLARE_EVENT_TABLE()
};
    
class MyGLCanvas: public wxGLCanvas, public wxScrollHelperNative
{
 public:
  MyGLCanvas(circuit* circ, wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxT("MyGLCanvas"));
  ~MyGLCanvas();
  void Render(); // function to draw canvas contents
  void OnMonitorSamplesChanged();
  void OnMonitorsChanged();
  void UpdateMinCanvasSize();
  void SetErrorMessage(wxString txt);
  void ClearErrorMessage();
 private:
  bool init;                         // has the GL context been initialised?
  circuit* c;
  int maxMonNameWidth;
  vector<GLCanvasMonitorTrace> mons;
  void InitGL();                     // function to initialise GL context
  void OnSize(wxSizeEvent& event);   // callback for when canvas is resized
  void OnPaint(wxPaintEvent& event); // callback for when canvas is exposed
  void OnMouse(wxMouseEvent& event); // callback for mouse events inside canvas
  void DrawInfoTextCentre(wxString txt, bool isError = false);
  void Redraw();
  wxString errorMessage;
  int scrollX, scrollY;
  int minXScale, maxXScale;
public:
  virtual void ScrollWindow(int dx, int dy, const wxRect* rect = (wxRect *)NULL);

  WX_FORWARD_TO_SCROLL_HELPER() // copied from wxScrolledWindow

private:
  DECLARE_EVENT_TABLE()
};





class AddMonitorsDialog: public wxDialog
{
public:
	AddMonitorsDialog(circuit* circ, wxWindow* parent, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
private:
	circuit* c;
	int oldMonCount;
	vector<outputinfo> availableOutputs;
	wxListBox *lbox;
	void OnOK(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

class DelMonitorsDialog: public wxDialog
{
public:
	DelMonitorsDialog(circuit* circ, wxWindow* parent, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE);
private:
	circuit* c;
	wxListBox *lbox;
	void OnOK(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

class SwitchesCheckListBox: public wxCheckListBox
{
public:
	SwitchesCheckListBox(circuit* circ, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	~SwitchesCheckListBox();
	void OnCircuitChanged();
private:
	circuit* c;
	void OnSwitchChanged(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};


#endif /* gui_h */
