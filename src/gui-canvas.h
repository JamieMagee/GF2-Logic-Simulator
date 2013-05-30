#ifndef gui_canvas_h
#define gui_canvas_h

#include "circuit.h"
#include "gui-misc.h"
#include <wx/wx.h>
#include <wx/glcanvas.h>

int GetGlutTextWidth(wxString txt, void *font=NULL);
void DrawGlutText(int x, int y, wxString txt, void *font=NULL);

class MyGLCanvas;

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



#endif /* gui_canvas_h */
