#include "gui.h"
#include <GL/glut.h>
#include "wx_icon.xpm"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "scanner.h"
#include "parser.h"
#include "gui-devices.h"

using namespace std;

// Get the width in pixels of some text when drawn using a particular font (font defaults to GLUT_BITMAP_HELVETICA_12)
int GetGlutTextWidth(wxString txt, void *font)
{
	int width = 0;
	if (!font) font = GLUT_BITMAP_HELVETICA_12;
	for (int i = 0; i < txt.Len(); i++)
		width += glutBitmapWidth(font, txt[i]);
	return width;
}

// Draws some text at the given position (font defaults to GLUT_BITMAP_HELVETICA_12)
void DrawGlutText(int x, int y, wxString txt, void *font)
{
	if (!font) font = GLUT_BITMAP_HELVETICA_12;
	int xMove = 0, yMove = 0;
	/* If x or y are outside the viewport, call glRasterPos2f with a position inside
	 * the viewport, then move it using glBitmap (as described in glRasterPos man page).
	 * Otherwise the text doesn't get rendered.
	 * Assuming viewport starts at (0,0) and only implemented for positions to the left of
	 * or above (0,0) for now. */
	if (x<0) xMove = x;
	if (y<0) yMove = y;
	glRasterPos2f(x-xMove, y-yMove);
	if (xMove || yMove) glBitmap (0, 0, 0, 0, xMove, yMove, NULL);
	for (int i = 0; i < txt.Len(); i++)
		glutBitmapCharacter(font, txt[i]);
}

// Call glVertex for the positions of the corners of a wxRect
// This makes drawing boxes a bit simpler
void wxRect_GlVertex(const wxRect& r)
{
	glVertex2i(r.x, r.y);
	glVertex2i(r.x+r.width, r.y);
	glVertex2i(r.x+r.width, r.y+r.height);
	glVertex2i(r.x, r.y+r.height);
}

// GLCanvasMonitorTrace - class to handle drawing of one monitor trace
GLCanvasMonitorTrace::GLCanvasMonitorTrace() :
	monId(-1), c(NULL), monName(wxT("")), monNameWidth(0), geometrySet(false)
{}

GLCanvasMonitorTrace::GLCanvasMonitorTrace(int newMonId, circuit* circ) :
	geometrySet(false)
{
	c = circ;
	SetMonitorId(newMonId);
}

int GLCanvasMonitorTrace::GetMonitorId()
{
	return monId;
}

void GLCanvasMonitorTrace::SetMonitorId(int newMonId)
{
	monId = newMonId;
	UpdateName();
}

void GLCanvasMonitorTrace::OnMonitorSamplesChanged(int totalCycles_new, int continuedCycles_new)
{
	totalCycles = totalCycles_new;
	continuedCycles = continuedCycles_new;
}

void GLCanvasMonitorTrace::UpdateName()
{
	if (!c)
	{
		monName = wxT("");
		monNameWidth = 0;
		return;
	}
	monName = wxString(c->mmz()->getsignalstring(monId).c_str(), wxConvUTF8);
	monNameWidth = GetGlutTextWidth(monName, GLUT_BITMAP_HELVETICA_12);
}

int GLCanvasMonitorTrace::GetNameWidth()
{
	return monNameWidth;
}

void GLCanvasMonitorTrace::SetGeometry(int xOffset_new, int yCentre_new, double xScale_new, int sigHeight_new, int padding_new, int spacing_new, int xBgName_new, int axisLabelInterval_new)
{
	geometrySet = true;
	xOffset = xOffset_new;
	yCentre = yCentre_new;
	xScale = xScale_new;
	sigHeight = int(sigHeight_new/2)*2;// make sure this is even, so that the trace is symmetrical about the horizontal centre line
	padding = padding_new;
	spacing = spacing_new;
	xBgName = xBgName_new;
	axisLabelInterval = axisLabelInterval_new;
}

void GLCanvasMonitorTrace::Draw(MyGLCanvas *canvas, const wxRect& visibleRegion)
{
	if (!c || !canvas || monId<0 || monId>=c->mmz()->moncount() || !geometrySet) return;

	// If this monitor was added after the simulation was run, it might have no data even though c->GetTotalCycles() is greater than zero
	int sampleCount = c->mmz()->getsamplecount(monId);
	if (!sampleCount)
	{
		glColor4f(0.8, 0.0, 0.0, 1.0);
		if (xOffset < visibleRegion.x+xBgName)
		{
			DrawGlutText(xBgName, yCentre-5, _("No data"), GLUT_BITMAP_HELVETICA_12);
		}
		else
		{
			DrawGlutText(xOffset+10, yCentre-5, _("No data"), GLUT_BITMAP_HELVETICA_12);
		}
	}
	if (totalCycles > sampleCount)
		totalCycles = sampleCount;
	if (continuedCycles > sampleCount)
		continuedCycles = sampleCount;

	wxRect backgroundRegion(xOffset, yCentre-sigHeight/2-padding, ceil(xScale*totalCycles), sigHeight+padding*2);
	wxRect traceRegion = wxRect(xOffset, yCentre-sigHeight/2-padding-11, ceil(xScale*totalCycles), sigHeight+padding*2+11);// includes cycle numbers on the axis
	if (traceRegion.Intersect(visibleRegion).IsEmpty()) return;
	wxRect clippedbg = backgroundRegion;
	clippedbg.Intersect(visibleRegion);

	// background colour
	glBegin(GL_QUADS);
	glColor4f(0.0, 0.5, 0.0, 0.08);
	wxRect_GlVertex(clippedbg);
	glEnd();

	// border
	glColor4f(0.0, 0.7, 0.0, 0.4);
	glBegin(GL_LINE_LOOP);
	wxRect_GlVertex(backgroundRegion);
	glEnd();

	// actual signal trace
	if (xScale>5) glLineWidth(2);
	glBegin(GL_LINE_STRIP);
	glColor4f(0.0, 0.8, 0.0, 1.0);
	int y1, y2, i;
	asignal s;
	int firstCycle = 0;
	int cycleLimit = totalCycles;
	if (xOffset < visibleRegion.x)
		firstCycle = int((visibleRegion.x-xOffset)/xScale);
	if (xOffset + totalCycles*xScale > visibleRegion.x+visibleRegion.width)
		cycleLimit = int((visibleRegion.x+visibleRegion.width - xOffset)/xScale) + 1;
	if (cycleLimit>totalCycles)
		cycleLimit = totalCycles;
	int prevY = yCentre;
	for (i=firstCycle; i<cycleLimit; i++)
	{
		if (c->mmz()->getsignaltrace(monId, i, s))
		{
			if (s==low || s==rising)
				y1 = yCentre-sigHeight/2;
			else
				y1 = yCentre+sigHeight/2;
			if (s==low || s==falling)
				y2 = yCentre-sigHeight/2;
			else
				y2 = yCentre+sigHeight/2;
			if (y1!=prevY) glVertex2i(xOffset+xScale*i, y1);
			glVertex2i(xOffset+xScale*(i+1), y2);
			prevY = y2;
		}
	}
	glEnd();
	glLineWidth(1);

	// Draw cycle numbers on axis and dashed vertical lines for them
	firstCycle = (int(firstCycle/axisLabelInterval)-1) * axisLabelInterval;
	if (firstCycle<0)
		firstCycle = 0;
	cycleLimit = (int(cycleLimit/axisLabelInterval)+1) * axisLabelInterval;
	if (cycleLimit>totalCycles)
		cycleLimit = totalCycles;
	glLineStipple(2, 0xAAAA);
	glEnable(GL_LINE_STIPPLE);
	for (i=firstCycle; i<=cycleLimit; i+=axisLabelInterval)
	{
		if (i!=0 && i!=totalCycles)
		{
			glBegin(GL_LINE_STRIP);
			glColor4f(0.0, 0.0, 0.0, 0.2);
			glVertex2i(xOffset+xScale*i, yCentre-sigHeight/2-padding);
			glVertex2i(xOffset+xScale*i, yCentre+sigHeight/2+padding);
			glEnd();
		}
		glColor4f(0.0, 0.0, 0.0, 1.0);
		wxString labelText;
		labelText.Printf(wxT("%d"), i);
		int labelWidth = GetGlutTextWidth(labelText, GLUT_BITMAP_HELVETICA_10);
		DrawGlutText(xOffset+xScale*i - labelWidth/2, yCentre-sigHeight/2-padding-11, labelText, GLUT_BITMAP_HELVETICA_10);
	}
	glDisable(GL_LINE_STIPPLE);
}

void GLCanvasMonitorTrace::DrawName(MyGLCanvas *canvas, const wxRect& visibleRegion)
{
	if (!geometrySet) return;
	if (xOffset < visibleRegion.x+xBgName && c->mmz()->getsamplecount(monId))
	{
		glColor4f(0.0, 0.0, 0.0, 0.4);
		DrawGlutText(xBgName, yCentre-5, monName, GLUT_BITMAP_HELVETICA_12);
	}
	else
	{
		glColor4f(0.0, 0.0, 0.0, 1.0);
		DrawGlutText(xOffset-monNameWidth-5, yCentre-5, monName, GLUT_BITMAP_HELVETICA_12);
	}
}

// MyGLCanvas ////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
  EVT_SIZE(MyGLCanvas::OnSize)
  EVT_PAINT(MyGLCanvas::OnPaint)
  EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
END_EVENT_TABLE()
  
int wxglcanvas_attrib_list[5] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

MyGLCanvas::MyGLCanvas(circuit* circ, wxWindow *parent, wxWindowID id,
	const wxPoint& pos, const wxSize& size, long style, const wxString& name):
    wxGLCanvas(parent, id, pos, size, style, name, wxglcanvas_attrib_list),
	wxScrollHelperNative(this), scrollX(0), scrollY(0)
  // Constructor - initialises private variables
{
	init = false;
	c = circ;
	if (c)
	{
		c->monitorsChanged.Attach(this, &MyGLCanvas::OnMonitorsChanged);
		c->monitorSamplesChanged.Attach(this, &MyGLCanvas::OnMonitorSamplesChanged);
		c->monitorSamplesChanged.Attach(this, &MyGLCanvas::ClearErrorMessage);
		OnMonitorsChanged();
	}
	SetScrollRate(10,10);
	minXScale = 2;
	maxXScale = 50;
}

MyGLCanvas::~MyGLCanvas()
{
	if (c)
	{
		c->monitorsChanged.Detach(this);
		c->monitorSamplesChanged.Detach(this);
	}
}

void MyGLCanvas::Redraw()
{
	// Redraw the contents of the canvas
	// This might look bizarre, but it seems to work everywhere tested (linux, and a cross compiled executable running in wine)
	// Removing any of these calls mean the canvas is sometimes not redrawn when running in wine
	Refresh();
	Update();
	Refresh();
}

void MyGLCanvas::ScrollWindow(int dx, int dy, const wxRect *rect)
{
	// override ScrollWindow(), since MyGLCanvas does its own scrolling by offsetting all drawn points and clipping
	scrollX += dx;
	scrollY += dy;
	Redraw();
}

void MyGLCanvas::UpdateMinCanvasSize()
{
	int totalCycles = c->GetTotalCycles();
	if (!mons.size())
		totalCycles = 0;//If all monitors have been deleted, don't reserve horizontal space
	// Make sure all the traces fit in the canvas
	int xOffset = maxMonNameWidth+5;
	int maxXTextWidth = ceil(log10(totalCycles))*8;// estimate of max x axis scale text width
	SetVirtualSize(minXScale*totalCycles+15+xOffset+maxXTextWidth/2,50*mons.size()+10);
}

// Notify of a change to the number of displayed cycles
void MyGLCanvas::OnMonitorSamplesChanged()
{
	int totalCycles = c->GetTotalCycles();
	int continuedCycles = c->GetContinuedCycles();
	for (int i=0; i<mons.size(); i++)
	{
		mons[i].OnMonitorSamplesChanged(totalCycles, continuedCycles);
	}
	UpdateMinCanvasSize();
	// Scroll to the most recently simulated cycles
	Scroll(GetVirtualSize().GetWidth()-GetClientSize().GetWidth(),-1);
	Redraw();
}

// Notify of a change to the active monitors
void MyGLCanvas::OnMonitorsChanged()
{
	int monCount = c->mmz()->moncount();
	mons.resize(monCount);
	maxMonNameWidth = 0;
	int totalCycles = c->GetTotalCycles();
	int continuedCycles = c->GetContinuedCycles();
	for (int i=0; i<monCount; i++)
	{
		mons[i] = GLCanvasMonitorTrace(i, c);
		mons[i].OnMonitorSamplesChanged(totalCycles, continuedCycles);
		if (mons[i].GetNameWidth()>maxMonNameWidth)
			maxMonNameWidth = mons[i].GetNameWidth();
	}
	UpdateMinCanvasSize();
	Redraw();
}

void MyGLCanvas::Render()
{
	unsigned int i;

	SetCurrent();
	if (!init)
	{
		InitGL();
		init = true;
	}
	glClear(GL_COLOR_BUFFER_BIT);
	if (c->GetTotalCycles() > 0 && c->mmz()->moncount() > 0)
	{
		int totalCycles = c->GetTotalCycles();
		// Scale traces (within limits) to fit the size of the canvas
		int monCount = c->mmz()->moncount();
		int canvasHeight = GetClientSize().GetHeight();
		int canvasWidth = GetClientSize().GetWidth();
		int spacing = (canvasHeight-10)/monCount;
		if (spacing>200) spacing = 200;
		if (spacing<50) spacing = 50;
		int height = 0.8*(spacing-14);
		int xOffset = maxMonNameWidth+10;
		double xScale = double(canvasWidth-xOffset-10)/c->GetTotalCycles();
		if (xScale<minXScale) xScale = minXScale;
		if (xScale>50) xScale = 50;

		wxRect visibleRegion(0,0,canvasWidth,canvasHeight);

		// Work out the best interval to use between displayed cycle numbers on the x axis
		// Intervals will be 1, 2, or 5 * pow(10,n) cycles between displayed numbers, where n>=0
		int minAxisLabelIntervalPixels = ceil(ceil(log10(totalCycles))*8*2);
		int base = 1;
		int baseMultiples[] = {1,2,5};
		int axisLabelInterval = 1;
		while (axisLabelInterval<=totalCycles)
		{
			for (i=0; i<3; i++)
			{
				axisLabelInterval = baseMultiples[i]*base;
				if (axisLabelInterval*xScale >= minAxisLabelIntervalPixels)
					break;
			}
			if (axisLabelInterval*xScale >= minAxisLabelIntervalPixels)
				break;
			base *= 10;
		}
		// Draw the traces
		for (i=0; i<mons.size(); i++)
		{
			mons[i].SetGeometry(xOffset+scrollX, canvasHeight-scrollY-spacing*i-spacing/2, xScale, height, spacing*0.075, spacing, 10, axisLabelInterval);
			mons[i].Draw(this, visibleRegion);
			mons[i].DrawName(this, visibleRegion);
		}
		if (errorMessage != wxT(""))
		{
			DrawInfoTextCentre(errorMessage, true);
		}
	}
	else if (errorMessage != wxT(""))
	{
		DrawInfoTextCentre(errorMessage, true);
	}
	else if (c->netz()->devicelist()==NULL)
	{
		DrawInfoTextCentre(_("No circuit loaded"));
	}
	else if (c->mmz()->moncount()==0)
	{
		DrawInfoTextCentre(_("No monitors"));
	}
	else
	{
		DrawInfoTextCentre(_("No simulation results. Use the run button."));
	}

	// We've been drawing to the back buffer, flush the graphics pipeline and swap the back buffer to the front
	glFlush();
	SwapBuffers();
}

void MyGLCanvas::SetErrorMessage(wxString txt)
{
	// Set an error message to be displayed in the centre of the canvas
	errorMessage = txt;
	Redraw();
}

void MyGLCanvas::ClearErrorMessage()
{
	errorMessage = wxT("");
}

void MyGLCanvas::DrawInfoTextCentre(wxString txt, bool isError)
{
	// Draw a message in a box in the centre of the canvas
	// isError makes it a red box
	int canvasHeight = GetClientSize().GetHeight();
	int canvasWidth = GetClientSize().GetWidth();
	int textWidth = GetGlutTextWidth(txt);
	wxRect background(canvasWidth/2-textWidth/2-15, canvasHeight/2-15, textWidth+30, 30);
	if (isError) glColor4f(1.0, 0.85, 0.85, 0.95);
	else glColor4f(0.85, 0.85, 1.0, 0.95);
	glBegin(GL_QUADS);
	wxRect_GlVertex(background);
	glEnd();
	if (isError) glColor4f(0.7, 0.0, 0.0, 0.95);
	else glColor4f(0.0, 0.0, 0.7, 0.95);
	glBegin(GL_LINE_LOOP);
	wxRect_GlVertex(background);
	glEnd();
	glColor4f(0.0, 0.0, 0.0, 1.0);
	DrawGlutText(canvasWidth/2-textWidth/2, canvasHeight/2-4, txt);
}

void MyGLCanvas::InitGL()
  // Function to initialise the GL context
{
  int w, h;

  GetClientSize(&w, &h);
  SetCurrent();
  glDrawBuffer(GL_BACK);
  glClearColor(1.0, 1.0, 1.0, 0.0);
  glViewport(0, 0, (GLint) w, (GLint) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, w, 0, h, -1, 1); 
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
}

void MyGLCanvas::OnPaint(wxPaintEvent& event)
  // Callback function for when the canvas is exposed
{
	wxPaintDC dc(this); // required for correct refreshing under MS windows
	Render();
}

void MyGLCanvas::OnSize(wxSizeEvent& event)
  // Callback function for when the canvas is resized
{
  wxGLCanvas::OnSize(event); // required on some platforms
  init = false;
  Redraw();// required by some buggy nvidia graphics drivers, harmless on other platforms!
}

void MyGLCanvas::OnMouse(wxMouseEvent& event)
  // Callback function for mouse events inside the GL canvas
{
  wxString text;
  int w, h;;

  GetClientSize(&w, &h);
  if (event.ButtonDown()) text.Printf(wxT("Mouse button %d pressed at %d %d"), event.GetButton(), event.m_x, h-event.m_y);
  if (event.ButtonUp()) text.Printf(wxT("Mouse button %d released at %d %d"), event.GetButton(), event.m_x, h-event.m_y);
  if (event.Dragging()) text.Printf(wxT("Mouse dragged to %d %d"), event.m_x, h-event.m_y);
  if (event.Leaving()) text.Printf(wxT("Mouse left window at %d %d"), event.m_x, h-event.m_y);

  //if (event.ButtonDown() || event.ButtonUp() || event.Dragging() || event.Leaving()) Render(text);
}


// MyFrame ///////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU(wxID_EXIT, MyFrame::OnExit)
  EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
  EVT_MENU(wxID_OPEN, MyFrame::OnOpenFile)
  EVT_BUTTON(SIMCTRL_BUTTON_RUN_ID, MyFrame::OnButtonRun)
  EVT_BUTTON(SIMCTRL_BUTTON_CONT_ID, MyFrame::OnButtonContinue)
  EVT_BUTTON(MONITORS_ADD_BUTTON_ID, MyFrame::OnButtonAddMon)
  EVT_BUTTON(MONITORS_DEL_BUTTON_ID, MyFrame::OnButtonDelMon)
  EVT_BUTTON(DEVICES_EDIT_BUTTON_ID, MyFrame::OnButtonEditDevs)
END_EVENT_TABLE()
  
MyFrame::MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size,
		 names *names_mod, devices *devices_mod, monitor *monitor_mod, network *net_mod, long style):
  wxFrame(parent, wxID_ANY, title, pos, size, style)
  // Constructor - initialises pointers to names, devices and monitor classes, lays out widgets
  // using sizers
{
  SetIcon(wxIcon(wx_icon));

  if (names_mod == NULL || devices_mod == NULL || monitor_mod == NULL || net_mod == NULL) {
    cout << "Cannot operate GUI without names, devices, network and monitor classes" << endl;
    exit(1);
  }
	c = new circuit(names_mod, devices_mod, monitor_mod, net_mod);
  
	// Menu items
	wxMenu *fileMenu = new wxMenu;
	fileMenu->Append(wxID_OPEN);
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_ABOUT, _("&About"));
	fileMenu->Append(wxID_EXIT, _("&Quit"));
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, _("&File"));
	SetMenuBar(menuBar);

	// Everything is contained in a wxPanel for improved appearance in wine/windows, as
	// described in http://wiki.wxwidgets.org/WxFAQ
	wxPanel* mainPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer *topsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *leftsizer = new wxBoxSizer(wxVERTICAL);

	// Canvas for drawing monitor traces
	canvas = new MyGLCanvas(c, mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
	leftsizer->Add(canvas, 3, wxEXPAND | wxALL, 10);

	// Create the log textbox, mainly for displaying error messages from the parser, captures everything sent to cout
	// wxTE_DONTWRAP means that a horizontal scrollbar will be used instead of wrapping, so that the position of an error can be indicated correctly
	outputTextCtrl = new wxTextCtrl(mainPanel, OUTPUT_TEXTCTRL_ID, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);

	// Set log textbox to a monospace font, so that the position of an error can be indicated correctly
	wxTextAttr outputTextAttr = outputTextCtrl->GetDefaultStyle();
	outputTextAttr.SetFont(wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	outputTextCtrl->SetDefaultStyle(outputTextAttr);

	// Redirect all text sent to cout to the log textbox
	outputTextRedirect = new wxStreamToTextRedirector(outputTextCtrl);
	leftsizer->Add(outputTextCtrl, 1, wxEXPAND | wxALL, 10);
	topsizer->Add(leftsizer, 4, wxEXPAND | wxALL, 10);

	// Simulation controls box
	wxBoxSizer *sidesizer = new wxBoxSizer(wxVERTICAL);
	simctrls_container = new wxPanel(mainPanel, wxID_ANY);
	wxStaticBoxSizer *simctrls_sizer = new wxStaticBoxSizer(wxVERTICAL, simctrls_container, _("Simulation"));
	wxBoxSizer *simctrls_cycles_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *simctrls_button_sizer = new wxBoxSizer(wxHORIZONTAL);

	// Run and continue simulation buttons
	simctrl_run = new wxButton(simctrls_container, SIMCTRL_BUTTON_RUN_ID, _("Run"));
	simctrl_continue = new wxButton(simctrls_container, SIMCTRL_BUTTON_CONT_ID, _("Continue"));
	simctrls_button_sizer->Add(simctrl_run, 0, wxALL, 10);
	simctrls_button_sizer->Add(simctrl_continue, 0, wxALL, 10);
	// Simulation cycle number spinner
	simctrls_cycles_sizer->Add(new wxStaticText(simctrls_container, wxID_ANY, _("Cycles")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 10);
	spin = new wxSpinCtrl(simctrls_container, MY_SPINCNTRL_ID, wxString(wxT("42")));
	spin->SetRange(1,1000000);
	simctrls_cycles_sizer->Add(spin, 0, wxALL, 10);

	simctrls_sizer->Add(simctrls_cycles_sizer);
	simctrls_sizer->Add(simctrls_button_sizer);
	simctrls_container->SetSizerAndFit(simctrls_sizer);

	// Buttons to open add/remove monitor dialogs
	wxStaticBoxSizer *edit_sizer = new wxStaticBoxSizer(wxVERTICAL, mainPanel, _("Edit circuit"));
	monitors_add_btn = new wxButton(mainPanel, MONITORS_ADD_BUTTON_ID, _("Add monitors"));
	monitors_rem_btn = new wxButton(mainPanel, MONITORS_DEL_BUTTON_ID, _("Remove monitors"));
	edit_sizer->Add(monitors_add_btn, 0, (wxALL & ~wxBOTTOM) | wxEXPAND, 10);
	edit_sizer->Add(monitors_rem_btn, 0, wxLEFT | wxRIGHT | wxEXPAND, 10);
	edit_sizer->Add(new wxButton(mainPanel, DEVICES_EDIT_BUTTON_ID, _("Edit devices")), 0, (wxALL & ~wxTOP) | wxEXPAND, 10);

	// wxCheckListBox that allows switch states to be changed 
	wxStaticBoxSizer *switches_sizer = new wxStaticBoxSizer(wxVERTICAL, mainPanel, _("Switches"));
	switchesCtrl = new SwitchesCheckListBox(c, mainPanel, SWITCHES_CTRL_ID, wxDefaultPosition, wxDefaultSize, wxLB_NEEDED_SB);
	switches_sizer->Add(switchesCtrl, 1, wxEXPAND | wxALL, 10);


	sidesizer->Add(simctrls_container, 0, wxALL, 10);
	sidesizer->Add(edit_sizer, 0, wxEXPAND | wxALL, 10);
	sidesizer->Add(switches_sizer, 1, wxEXPAND | wxALL, 10);
	topsizer->Add(sidesizer, 0, wxEXPAND | wxALIGN_CENTER);

	SetSizeHints(400, 400);
	mainPanel->SetSizer(topsizer);

	c->circuitChanged.Attach(this, &MyFrame::UpdateControlStates);
	c->monitorsChanged.Attach(this, &MyFrame::UpdateControlStates);
	c->monitorSamplesChanged.Attach(this, &MyFrame::UpdateControlStates);
	UpdateControlStates();
}

MyFrame::~MyFrame()
{
	c->circuitChanged.Detach(this);
	c->monitorsChanged.Detach(this);
	c->monitorSamplesChanged.Detach(this);
	delete outputTextRedirect;
}

void MyFrame::OnExit(wxCommandEvent &event)
  // Callback for the exit menu item
{
  Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &event)
  // Callback for the about menu item
{
  wxMessageDialog about(this, _("Logic simulator\nIIA GF2 Team 8\n2013"), _("About Logsim"), wxICON_INFORMATION | wxOK);
  about.ShowModal();
}

void MyFrame::OnOpenFile(wxCommandEvent &event)
  // Callback for the File -> Open menu item
{
	wxFileDialog openFileDialog(this, _("Open logic circuit"), wxT("../examples/"), wxT(""), _("Logic circuit files (*.gf2)|*.gf2|All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // cancelled, don't open a file
	loadFile(openFileDialog.GetPath().mb_str());
}

bool MyFrame::loadFile(const char * filename)
// load a file (can be called by menu File->Open or for the command line argument)
{
	// Clear log window
	outputTextCtrl->ChangeValue(wxT(""));
	cout << "Loading file " << filename << endl;

	c->Clear();

	scanner *smz = new scanner(c->nmz(), filename);
	error *erz = new error(smz);
	parser *pmz = new parser(c->netz(), c->dmz(), c->mmz(), smz, erz);
	bool result = pmz->readin();

	if (result)
	{
		c->netz()->checknetwork(result);
	}

	if (!result)
	{
		cout << "Failed to load file" << endl;
		c->Clear();

		// scroll to the start of the output so that the first error message (which may have caused any subsequent error messages) is visible
		outputTextCtrl->ShowPosition(0);
	}

	c->circuitChanged.Trigger();
	c->monitorsChanged.Trigger();
	c->monitorSamplesChanged.Trigger();

	if (!result)
	{
		canvas->SetErrorMessage(_("Failed to load file"));
	}

	delete pmz;
	delete smz;
	delete erz;

	return result;
}

void MyFrame::UpdateControlStates()
{
	// Update enabled/disabled state of controls
	if (c->netz()->devicelist()==NULL)
	{
		// If there are no devices, disable simulation controls and add/remove monitor buttons
		simctrls_container->Disable();
		monitors_add_btn->Disable();
		monitors_rem_btn->Disable();
	}
	else
	{
		// The circuit contains some devices, so enable the simulation controls if all inputs are connected
		// It is assumed that unconnected inputs are listed by whichever bit of code has allowed them to exist in the circuit, this function just updates control stated
		bool ok;
		c->netz()->checknetwork(ok, true);
		if (ok)
			simctrls_container->Enable();
		else
			simctrls_container->Disable();
		// Only enable the add monitors button if some unmonitored outputs exist
		if (c->GetUnmonitoredOutputs())
			monitors_add_btn->Enable();
		else
			monitors_add_btn->Disable();
		// Only enable the remove monitors button if some monitors exist
		if (c->mmz()->moncount()>0)
			monitors_rem_btn->Enable();
		else
			monitors_rem_btn->Disable();
		// Disable the continue button if the run button has not been used first (so GetTotalCycles returns 0) or there are some new monitors (so some monitors do not contain any samples)
		bool someEmpty = (c->GetTotalCycles()==0);
		for (int i=0; i<c->mmz()->moncount(); i++)
		{
			if (c->mmz()->getsamplecount(i)==0)
				someEmpty = true;
		}
		if (someEmpty)
			simctrl_continue->Disable();
		else
			simctrl_continue->Enable();
		
	}
}

void MyFrame::OnButtonRun(wxCommandEvent &event)
  // Callback for the run simulation button
{
	c->Simulate(spin->GetValue(),true);
	simctrl_continue->Enable();
}

void MyFrame::OnButtonContinue(wxCommandEvent &event)
  // Callback for the continue simulation button
{
	c->Simulate(spin->GetValue(),false);
}

void MyFrame::OnButtonAddMon(wxCommandEvent& event)// "Add monitors" button clicked
{
	int oldMonCount = c->mmz()->moncount();
	AddMonitorsDialog *dlg = new AddMonitorsDialog(c, this, _("Add monitors"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	if (dlg->ShowModal()==wxID_OK && oldMonCount!=c->mmz()->moncount())
	{
		if (c->GetTotalCycles())
		{
			cout << wxString(wxPLURAL("Monitor added, run simulation again to see updated signals", "Monitors added, run simulation again to see updated signals", c->mmz()->moncount()-oldMonCount)).mb_str() << endl;
		}
	}
	dlg->Destroy();
}

void MyFrame::OnButtonDelMon(wxCommandEvent& event)// "Remove monitors" button clicked
{
	DelMonitorsDialog *dlg = new DelMonitorsDialog(c, this, _("Remove monitors"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	dlg->ShowModal();
	dlg->Destroy();
}

void MyFrame::OnButtonEditDevs(wxCommandEvent& event)// "Edit devices" button clicked
{
	DevicesDialog *dlg = new DevicesDialog(c, this, wxID_ANY, _("Edit devices"));
	dlg->ShowModal();
	dlg->Destroy();
	bool ok;
	c->netz()->checknetwork(ok);
	if (!ok) canvas->SetErrorMessage(_("Circuit contains unconnected inputs"));
	else canvas->ClearErrorMessage();
	UpdateControlStates();
}



AddMonitorsDialog::AddMonitorsDialog(circuit* circ, wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
	wxDialog(parent, wxID_ANY, title, pos, size, style)
{
	SetIcon(wxIcon(wx_icon));

	c = circ;
	oldMonCount = c->mmz()->moncount();

	c->GetUnmonitoredOutputs(&availableOutputs);
	sort(availableOutputs.begin(), availableOutputs.end(), outputinfo_namestrcmp);

	wxArrayString displayedOutputs;
	displayedOutputs.Alloc(availableOutputs.size());
	for (vector<outputinfo>::iterator it=availableOutputs.begin(); it!=availableOutputs.end(); ++it)
	{
		displayedOutputs.Add(wxString(it->namestr.c_str(), wxConvUTF8));
	}

	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
	topsizer->Add(new wxStaticText(this, wxID_ANY, _("Select output(s) to monitor:")), 0, wxLEFT | wxRIGHT | wxTOP, 10);
	lbox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, displayedOutputs, wxLB_EXTENDED | wxLB_NEEDED_SB);
	topsizer->Add(lbox, 1, wxALL | wxEXPAND, 10);
	topsizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
	SetSizerAndFit(topsizer);
}

void AddMonitorsDialog::OnOK(wxCommandEvent& event)
{
	wxArrayInt selections;
	lbox->GetSelections(selections);
	int num = selections.GetCount();
	bool ok;
	for (int i=0; i<num; i++)
	{
		c->mmz()->makemonitor(availableOutputs[selections[i]].devname, availableOutputs[selections[i]].outpname, ok);
	}
	if (c->mmz()->moncount() != oldMonCount)
	{
		c->monitorsChanged.Trigger();
	}
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(AddMonitorsDialog, wxDialog)
	EVT_BUTTON(wxID_OK, AddMonitorsDialog::OnOK)
END_EVENT_TABLE()


DelMonitorsDialog::DelMonitorsDialog(circuit* circ, wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
	wxDialog(parent, wxID_ANY, title, pos, size, style)
{
	SetIcon(wxIcon(wx_icon));
	c = circ;

	// Copy monitor names into a wxArrayString to pass to the listbox
	int monCount = c->mmz()->moncount();
	wxArrayString monitorList;
	monitorList.Alloc(monCount);
	for (int i=0; i<monCount; i++)
	{
		monitorList.Add(wxString(c->mmz()->getsignalstring(i).c_str(), wxConvUTF8));
	}

	// Create controls
	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
	topsizer->Add(new wxStaticText(this, wxID_ANY, _("Select monitors to remove:")), 0, wxLEFT | wxRIGHT | wxTOP, 10);
	lbox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, monitorList, wxLB_EXTENDED | wxLB_NEEDED_SB);
	topsizer->Add(lbox, 1, wxALL | wxEXPAND, 10);
	topsizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
	SetSizerAndFit(topsizer);
}

void DelMonitorsDialog::OnOK(wxCommandEvent& event)
{
	wxArrayInt selections;
	lbox->GetSelections(selections);
	bool ok;
	monitor* mmz = c->mmz();
	name dev, outp;
	// Remove selected monitors
	// Note loop direction - remmonitor deletes an entry and shifts monitors into the space, changing the IDs of later monitors. Therefore remove monitors with a higher ID (towards the end of the listbox) first.
	for (int i=selections.GetCount()-1; i>=0; i--)
	{
		mmz->getmonname(selections[i], dev, outp);
		mmz->remmonitor(dev, outp, ok);
	}
	if (selections.GetCount())
		c->monitorsChanged.Trigger();

	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(DelMonitorsDialog, wxDialog)
	EVT_BUTTON(wxID_OK, DelMonitorsDialog::OnOK)
END_EVENT_TABLE()


SwitchesCheckListBox::SwitchesCheckListBox(circuit* circ, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: wxCheckListBox(parent, id, pos, size, 0, NULL, style & ~wxLB_SORT)
{
	c = circ;
	if (c)
	{
		c->circuitChanged.Attach(this, &SwitchesCheckListBox::OnCircuitChanged);
	}
	OnCircuitChanged();
}

SwitchesCheckListBox::~SwitchesCheckListBox()
{
	if (c)
	{
		c->circuitChanged.Detach(this);
	}
}

void SwitchesCheckListBox::OnCircuitChanged()
{
	if (!c) return;

	wxArrayString switchNames;
	devlink d = c->netz()->devicelist();
	while (d!=NULL)
	{
		if (d->kind == aswitch)
		{
			switchNames.Add(wxString(c->nmz()->getnamestring(d->id).c_str(), wxConvUTF8));
		}
		d = d->next;
	}
	Set(switchNames);
	d = c->netz()->devicelist();
	int i = 0;
	while (d!=NULL)
	{
		if (d->kind == aswitch)
		{
			Check(i, d->swstate==high);
			i++;
		}
		d = d->next;
	}
}

void SwitchesCheckListBox::OnSwitchChanged(wxCommandEvent& event)
{
	if (!c) return;

	int changedI = event.GetInt();
	int i = 0;
	devlink targetD = NULL, d = c->netz()->devicelist();
	while (d!=NULL)
	{
		if (d->kind == aswitch)
		{
			if (i==changedI)
				targetD = d;
			i++;
		}
		d = d->next;
	}
	if (targetD==NULL)
	{
		wxMessageDialog dlg(this, _("Tried to change unknown switch"), _("Error"), wxCANCEL | wxICON_ERROR);
		dlg.ShowModal();
		return;
	}
	if (IsChecked(changedI))
		targetD->swstate = high;
	else
		targetD->swstate = low;

	c->circuitChanged.Trigger();
}

BEGIN_EVENT_TABLE(SwitchesCheckListBox, wxCheckListBox)
	EVT_CHECKLISTBOX(wxID_ANY, SwitchesCheckListBox::OnSwitchChanged)
END_EVENT_TABLE()
