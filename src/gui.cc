#include "gui.h"
#include <GL/glut.h>
#include "wx_icon.xpm"
#include <iostream>
#include <vector>
#include <cmath>
#include "scanner.h"
#include "parser.h"

using namespace std;

int GetGlutTextWidth(wxString txt, void *font)
{
	int width = 0;
	if (!font) font = GLUT_BITMAP_HELVETICA_12;
	for (int i = 0; i < txt.Len(); i++)
		width += glutBitmapWidth(font, txt[i]);
	return width;
}

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

void wxRect_GlVertex(const wxRect& r)
{
	glVertex2i(r.x, r.y);
	glVertex2i(r.x+r.width, r.y);
	glVertex2i(r.x+r.width, r.y+r.height);
	glVertex2i(r.x, r.y+r.height);
}

// GLCanvasMonitorTrace - class to handle drawing of one monitor trace
GLCanvasMonitorTrace::GLCanvasMonitorTrace() :
	monId(-1), mmz(NULL), nmz(NULL), monName(wxT("")), monNameWidth(0), geometrySet(false)
{}

GLCanvasMonitorTrace::GLCanvasMonitorTrace(int newMonId, monitor *monitor_mod, names *names_mod) :
	geometrySet(false)
{
	SetMonitorId(newMonId);
	SetModules(monitor_mod, names_mod);
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

void GLCanvasMonitorTrace::SetModules(monitor *monitor_mod, names *names_mod)
{
	mmz = monitor_mod;
	nmz = names_mod;
}

void GLCanvasMonitorTrace::SimulationRun(int totalCycles_new, int continuedCycles_new)
{
	totalCycles = totalCycles_new;
	continuedCycles = continuedCycles_new;
}

void GLCanvasMonitorTrace::UpdateName()
{
	if (monId<0 || !nmz || !mmz)
	{
		monName = wxT("");
		monNameWidth = 0;
		return;
	}
	name dev, outp;
	mmz->getmonname(monId, dev, outp);
	if (dev==blankname)
	{
		monName = wxT("");
		monNameWidth = 0;
		return;
	}
	monName = wxString(nmz->getnamestring(dev).c_str(), wxConvUTF8);
	if (outp!=blankname)
		monName += wxT(".") + wxString(nmz->getnamestring(outp).c_str(), wxConvUTF8);
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
	if (!mmz || !canvas || monId<0 || monId>=mmz->moncount()) return;

	int canvasHeight = canvas->GetClientSize().GetHeight();

	wxRect backgroundRegion(xOffset, yCentre-sigHeight/2-padding, ceil(xScale*totalCycles), sigHeight+padding*2);
	wxRect traceRegion = backgroundRegion.Intersect(visibleRegion);// this will be slightly different when cycle numbers are added to the axis
	if (traceRegion.IsEmpty()) return;
	wxRect clippedbg = backgroundRegion.Intersect(visibleRegion);

	// background colour
	glBegin(GL_QUADS);
	glColor4f(0.0, 0.5, 0.0, 0.08);
	wxRect_GlVertex(backgroundRegion.Intersect(visibleRegion));
	glEnd();

	// border
	glColor4f(0.0, 0.7, 0.0, 0.4);
	if (clippedbg==backgroundRegion)
	{
		// if the whole backgroundRegion is in the visible region, draw the whole border
		glBegin(GL_LINE_LOOP);
		wxRect_GlVertex(backgroundRegion);
		glEnd();
	}
	else
	{
		// otherwise just draw part of it
		glBegin(GL_LINES);
		// horizontal lines
		glVertex2i(clippedbg.x, clippedbg.y);
		glVertex2i(clippedbg.x+clippedbg.width, clippedbg.y);
		glVertex2i(clippedbg.x, clippedbg.y+clippedbg.height);
		glVertex2i(clippedbg.x+clippedbg.width, clippedbg.y+clippedbg.height);
		// vertical lines if they are in the visible region
		if (backgroundRegion.x >= visibleRegion.x)
		{
			glVertex2i(clippedbg.x, clippedbg.y+1);
			glVertex2i(clippedbg.x, clippedbg.y+clippedbg.height-1);
		}
		if (backgroundRegion.x+backgroundRegion.width <= visibleRegion.x+visibleRegion.width)
		{
			glVertex2i(clippedbg.x+clippedbg.width, clippedbg.y+1);
			glVertex2i(clippedbg.x+clippedbg.width, clippedbg.y+clippedbg.height-1);
		}
		glEnd();
	}

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
		if (mmz->getsignaltrace(monId, i, s))
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
	if (xOffset < visibleRegion.x+xBgName)
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

MyGLCanvas::MyGLCanvas(wxWindow *parent, wxWindowID id, monitor* monitor_mod, names* names_mod,
	const wxPoint& pos, const wxSize& size, long style, const wxString& name):
    wxGLCanvas(parent, id, pos, size, style, name, wxglcanvas_attrib_list),
	wxScrollHelperNative(this), scrollX(0), scrollY(0)
  // Constructor - initialises private variables
{
	mmz = monitor_mod;
	nmz = names_mod;
	init = false;
	continuedCycles = totalCycles = 0;
	MonitorsChanged();
	SetScrollRate(10,10);
	minXScale = 2;
	maxXScale = 50;
}

void MyGLCanvas::ScrollWindow(int dx, int dy, const wxRect *rect)
{
	// override ScrollWindow(), since MyGLCanvas does its own scrolling by offsetting all drawn points and clipping
	scrollX += dx;
	scrollY += dy;
	Refresh();
}

void MyGLCanvas::UpdateMinCanvasSize()
{
	// Make sure all the traces fit in the canvas
	int xOffset = maxMonNameWidth+5;
	int maxXTextWidth = ceil(log10(totalCycles))*8;// estimate of max x axis scale text width
	SetVirtualSize(minXScale*totalCycles+15+xOffset+maxXTextWidth/2,50*mons.size());
}

// Notify of a change to the number of displayed cycles
void MyGLCanvas::SimulationRun(int totalCycles_new, int continuedCycles_new)
{
	totalCycles = totalCycles_new;
	continuedCycles = continuedCycles_new;
	for (int i=0; i<mons.size(); i++)
	{
		mons[i].SimulationRun(totalCycles, continuedCycles);
	}
	Render();
	UpdateMinCanvasSize();
	// Scroll to the most recently simulated cycles
	Scroll(GetVirtualSize().GetWidth()-GetClientSize().GetWidth(),-1);
}

// Notify of a change to the active monitors
void MyGLCanvas::MonitorsChanged()
{
	int monCount = mmz->moncount();
	mons.resize(monCount);
	maxMonNameWidth = 0;
	for (int i=0; i<monCount; i++)
	{
		mons[i].SetMonitorId(i);
		mons[i].SetModules(mmz, nmz);
		mons[i].SimulationRun(totalCycles, continuedCycles);
		if (mons[i].GetNameWidth()>maxMonNameWidth)
			maxMonNameWidth = mons[i].GetNameWidth();
	}
	UpdateMinCanvasSize();
}


// copied from wxScrolledWindow:
#ifdef __WXMSW__
WXLRESULT wxScrolledWindow::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam,WXLPARAM lParam)
{
    WXLRESULT rc = wxPanel::MSWWindowProc(nMsg, wParam, lParam);
#ifndef __WXWINCE__
    // we need to process arrows ourselves for scrolling
    if ( nMsg == WM_GETDLGCODE )
    {
        rc |= DLGC_WANTARROWS;
    }
#endif
    return rc;
}
#endif // __WXMSW__
// end copied from wxScrolledWindow


void MyGLCanvas::Render(wxString text)
{
	unsigned int i;

	SetCurrent();
	if (!init)
	{
		InitGL();
		init = true;
	}
	glClear(GL_COLOR_BUFFER_BIT);
	if ((totalCycles > 0) && (mmz->moncount() > 0))
	{
		int monCount = mmz->moncount();
		int canvasHeight = GetClientSize().GetHeight();
		int canvasWidth = GetClientSize().GetWidth();
		int spacing = (canvasHeight-10)/monCount;
		if (spacing>200) spacing = 200;
		if (spacing<50) spacing = 50;
		int height = 0.8*(spacing-14);

		int xOffset = maxMonNameWidth+10;
		wxRect visibleRegion(0,0,canvasWidth,canvasHeight);

		double xScale = double(canvasWidth-xOffset-10)/totalCycles;
		if (xScale<minXScale) xScale = minXScale;
		if (xScale>50) xScale = 50;

		// Work out the best interval to use between displayed cycle numbers on the x axis
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
	}
	else if (mmz->moncount()==0)
	{
		glColor3f(0.5, 0.0, 0.0);
		DrawGlutText(5, 10, wxT("No monitors"));
	}
	else
	{
		glColor3f(0.8, 0.0, 0.0);
		DrawGlutText(5, 10, wxT("No simulation results to display"));
	}

	// We've been drawing to the back buffer, flush the graphics pipeline and swap the back buffer to the front
	glFlush();
	SwapBuffers();
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
  int w, h;
  wxString text;

  wxPaintDC dc(this); // required for correct refreshing under MS windows
  GetClientSize(&w, &h);
  text.Printf(wxT("Canvas redrawn by OnPaint callback, canvas size is %d by %d"), w, h);
  Render(text);
}

void MyGLCanvas::OnSize(wxSizeEvent& event)
  // Callback function for when the canvas is resized
{
  wxGLCanvas::OnSize(event); // required on some platforms
  init = false;
  Refresh(); // required by some buggy nvidia graphics drivers,
  Update();  // harmless on other platforms!
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

  if (event.ButtonDown() || event.ButtonUp() || event.Dragging() || event.Leaving()) Render(text);
}


// MyFrame ///////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU(wxID_EXIT, MyFrame::OnExit)
  EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
  EVT_MENU(wxID_OPEN, MyFrame::OnOpenFile)
  EVT_BUTTON(SIMCTRL_BUTTON_RUN_ID, MyFrame::OnButtonRun)
  EVT_BUTTON(SIMCTRL_BUTTON_CONT_ID, MyFrame::OnButtonContinue)
  EVT_SPINCTRL(MY_SPINCNTRL_ID, MyFrame::OnSpin)
  EVT_TEXT_ENTER(MY_TEXTCTRL_ID, MyFrame::OnText)
END_EVENT_TABLE()
  
MyFrame::MyFrame(wxWindow *parent, const wxString& title, const wxPoint& pos, const wxSize& size,
		 names *names_mod, devices *devices_mod, monitor *monitor_mod, network *net_mod, long style):
  wxFrame(parent, wxID_ANY, title, pos, size, style)
  // Constructor - initialises pointers to names, devices and monitor classes, lays out widgets
  // using sizers
{
  SetIcon(wxIcon(wx_icon));

  nmz = names_mod;
  dmz = devices_mod;
  mmz = monitor_mod;
  netz = net_mod;
  if (nmz == NULL || dmz == NULL || mmz == NULL || netz == NULL) {
    cout << "Cannot operate GUI without names, devices, network and monitor classes" << endl;
    exit(1);
  }

  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_OPEN);
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_ABOUT, wxT("&About"));
  fileMenu->Append(wxID_EXIT, wxT("&Quit"));
  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, wxT("&File"));
  SetMenuBar(menuBar);

	wxBoxSizer *topsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *leftsizer = new wxBoxSizer(wxVERTICAL);

	canvas = new MyGLCanvas(this, wxID_ANY, monitor_mod, names_mod, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
	leftsizer->Add(canvas, 3, wxEXPAND | wxALL, 10);

	// Create the log textbox, mainly for displaying error messages from the parser
	// Captures everything sent to cout
	// wxTE_DONTWRAP means that a horizontal scrollbar will be used instead of wrapping, so that the position of an error can be indicated correctly
	outputTextCtrl = new wxTextCtrl(this, OUTPUT_TEXTCTRL_ID, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);

	// Set to a monospace font, so that the position of an error can be indicated correctly
	wxTextAttr outputTextAttr = outputTextCtrl->GetDefaultStyle();
	outputTextAttr.SetFont(wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
	outputTextCtrl->SetDefaultStyle(outputTextAttr);

	// Redirect all text sent to cout to the outputTextCtrl textbox
	outputTextRedirect = new wxStreamToTextRedirector(outputTextCtrl);
	leftsizer->Add(outputTextCtrl, 1, wxEXPAND | wxALL, 10);
	topsizer->Add(leftsizer, 4, wxEXPAND | wxALL, 10);


	wxBoxSizer *sidesizer = new wxBoxSizer(wxVERTICAL);
	wxStaticBoxSizer *simctrls_sizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Simulation"));
	wxBoxSizer *simctrls_cycles_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *simctrls_button_sizer = new wxBoxSizer(wxHORIZONTAL);

	simctrls_button_sizer->Add(new wxButton(this, SIMCTRL_BUTTON_RUN_ID, wxT("Run")), 0, wxALL, 10);
	simctrls_button_sizer->Add(new wxButton(this, SIMCTRL_BUTTON_CONT_ID, wxT("Continue")), 0, wxALL, 10);
	simctrls_cycles_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Cycles")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 10);
	spin = new wxSpinCtrl(this, MY_SPINCNTRL_ID, wxString(wxT("31")));
	spin->SetRange(1,1000000);
	simctrls_cycles_sizer->Add(spin, 0, wxALL, 10);

	simctrls_sizer->Add(simctrls_cycles_sizer);
	simctrls_sizer->Add(simctrls_button_sizer);

	wxStaticBoxSizer *monitors_sizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Monitors"));

	wxStaticBoxSizer *switches_sizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Switches"));


	sidesizer->Add(simctrls_sizer, 0, wxALL, 10);
	sidesizer->Add(monitors_sizer, 1, wxEXPAND | wxALL, 10);
	sidesizer->Add(switches_sizer, 1, wxEXPAND | wxALL, 10);
	//sidesizer->Add(new wxTextCtrl(this, MY_TEXTCTRL_ID, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0 , wxALL, 10);
	topsizer->Add(sidesizer, 0, wxEXPAND | wxALIGN_CENTER);

	SetSizeHints(400, 400);
	SetSizer(topsizer);
}

MyFrame::~MyFrame()
{
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
  wxMessageDialog about(this, wxT("Example wxWidgets GUI\nAndrew Gee\nFebruary 2011"), wxT("About Logsim"), wxICON_INFORMATION | wxOK);
  about.ShowModal();
}

void MyFrame::OnOpenFile(wxCommandEvent &event)
  // Callback for the File -> Open menu item
{
	wxFileDialog openFileDialog(this, wxT("Open logic circuit"), wxT(""), wxT(""), wxT("Logic circuit files (*.gf2)|*.gf2|All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // cancelled, don't open a file
	loadFile(openFileDialog.GetPath().mb_str());
}

void MyFrame::clearCircuit()
{
	//TODO
}

bool MyFrame::loadFile(const char * filename)
// load a file (can be called by menu File->Open or for the command line argument)
{
	// Clear log window
	outputTextCtrl->ChangeValue(wxT(""));
	cout << "Loading file " << filename << endl;

	clearCircuit();
	scanner *smz = new scanner(nmz, filename);
	parser *pmz = new parser(netz, dmz, mmz, smz);
	bool result = pmz->readin();

	//TODO: maybe display a messagebox here or disable a few UI controls (like the Run button) if reading failed
	if (!result)
		cout << "Failed to load file" << endl;

	canvas->MonitorsChanged();

	if (!result)
	{
		// scroll to the start of the output so that the first error message (which may have caused any subsequent error messages) is visible
		outputTextCtrl->ShowPosition(0);
	}
	return result;
}

void MyFrame::OnButtonRun(wxCommandEvent &event)
  // Callback for the run simulation button
{
	int n, ncycles;
	totalCycles = 0;
	mmz->resetmonitor();
	runnetwork(spin->GetValue());
	canvas->SimulationRun(totalCycles, continuedCycles);
}

void MyFrame::OnButtonContinue(wxCommandEvent &event)
  // Callback for the run simulation button
{
	int n, ncycles;
	runnetwork(spin->GetValue());
	canvas->SimulationRun(totalCycles, continuedCycles);
}

void MyFrame::OnSpin(wxSpinEvent &event)
  // Callback for the spin control
{
  wxString text;

  text.Printf(wxT("New spinctrl value %d"), event.GetPosition());
  canvas->Render(text);
}

void MyFrame::OnText(wxCommandEvent &event)
  // Callback for the text entry field
{
  wxString text;

  text.Printf(wxT("New text entered %s"), event.GetString().c_str());
  canvas->Render(text);
}

void MyFrame::runnetwork(int ncycles)
  // Function to run the network
{
	bool ok = true;
	int n = ncycles;
	continuedCycles = 0;

	while ((n > 0) && ok)
	{
		dmz->executedevices (ok);
		if (ok)
		{
			n--;
			totalCycles++;
			continuedCycles++;
			mmz->recordsignals();
		}
		else
		{
			cout << "Error: network is oscillating" << endl;
		}
	}
	/*if (ok) totalCycles = totalCycles + ncycles;
	else totalCycles = 0;*/
}
