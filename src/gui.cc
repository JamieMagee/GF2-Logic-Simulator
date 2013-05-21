#include "gui.h"
#include <GL/glut.h>
#include "wx_icon.xpm"
#include <iostream>
#include <vector>
#include "scanner.h"
#include "parser.h"

using namespace std;

// MyGLCanvas ////////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
  EVT_SIZE(MyGLCanvas::OnSize)
  EVT_PAINT(MyGLCanvas::OnPaint)
  EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
END_EVENT_TABLE()
  
int wxglcanvas_attrib_list[5] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

MyGLCanvas::MyGLCanvas(wxWindow *parent, wxWindowID id, monitor* monitor_mod, names* names_mod,
		       const wxPoint& pos, const wxSize& size, long style, const wxString& name):
    wxGLCanvas(parent, id, pos, size, style, name, wxglcanvas_attrib_list)
  // Constructor - initialises private variables
{
  mmz = monitor_mod;
  nmz = names_mod;
  init = false;
  continuedCycles = totalCycles = 0;
}

void MyGLCanvas::DrawSignalTrace(int xOffset, int yOffset, float xScale, int height, int padding, int mon, int cycles)
{
	glBegin(GL_QUADS);
	glColor4f(0.0, 0.5, 0.0, 0.05);
	glVertex2f(xOffset, yOffset-height/2-padding);
	glVertex2f(xOffset, yOffset+height/2+padding);
	glVertex2f(xOffset+xScale*cycles, yOffset+height/2+padding);
	glVertex2f(xOffset+xScale*cycles, yOffset-height/2-padding);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glColor4f(0.0, 0.7, 0.0, 0.4);
	glVertex2f(xOffset, yOffset-height/2-padding);
	glVertex2f(xOffset, yOffset+height/2+padding);
	glVertex2f(xOffset+xScale*cycles, yOffset+height/2+padding);
	glVertex2f(xOffset+xScale*cycles, yOffset-height/2-padding);
	glEnd();

	if (xScale>4) glLineWidth(2);
	glBegin(GL_LINE_STRIP);
	glColor4f(0.0, 0.8, 0.0, 1.0);
	int y1, y2, i;
	asignal s;
	for (i=0; i<cycles; i++)
	{
		if (mmz->getsignaltrace(mon, i, s))
		{
			if (s==low || s==rising)
				y1 = yOffset-height/2;
			else
				y1 = yOffset+height/2;
			if (s==low || s==falling)
				y2 = yOffset-height/2;
			else
				y2 = yOffset+height/2;
			glVertex2f(xOffset+xScale*i, y1);
			glVertex2f(xOffset+xScale*(i+1), y2);
		}
	}
	glEnd();
	glLineWidth(1);
}

void MyGLCanvas::DrawText(int x, int y, wxString txt, void *font)
{
	int width = 0;
	if (!font) font = GLUT_BITMAP_HELVETICA_12;
	glRasterPos2f(x, y);
	for (int i = 0; i < txt.Len(); i++)
		glutBitmapCharacter(font, txt[i]);
}

int MyGLCanvas::GetTextWidth(wxString txt, void *font)
{
	int width = 0;
	if (!font) font = GLUT_BITMAP_HELVETICA_12;
	for (int i = 0; i < txt.Len(); i++)
		width += glutBitmapWidth(font, txt[i]);
	return width;
}

void MyGLCanvas::SimulationRun(int totalCycles_new, int continuedCycles_new)
{
	totalCycles = totalCycles_new;
	continuedCycles = continuedCycles_new;
	Render();
}

void MyGLCanvas::Render(wxString text)
  // Draws canvas contents - the following example writes the string "example text" onto the canvas
  // and draws a signal trace. The trace is artificial if the simulator has not yet been run.
  // When the simulator is run, the number of cycles is passed as a parameter and the first monitor
  // trace is displayed.
{
	float y;
	unsigned int i;
	asignal s;

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
		int mon, height=20, spacing=30;
		name dev, outp;
		vector<wxString> monNames;
		vector<int> monNameWidths;
		int maxNameWidth = 1;
		int canvasHeight = GetVirtualSize().GetHeight();
		int canvasWidth = GetVirtualSize().GetWidth();
		spacing = canvasHeight/monCount;
		if (spacing>200) spacing = 200;
		if (spacing<10) spacing = 10;
		height = 0.8*spacing;
		for (mon=0; mon<monCount; mon++)
		{
			mmz->getmonname(mon, dev, outp);
			wxString monName(nmz->getnamestring(dev).c_str(), wxConvUTF8);
			if (outp!=blankname)
				monName += wxT(".") + wxString(nmz->getnamestring(outp).c_str(), wxConvUTF8);
			monNames.push_back(monName);
			int w = GetTextWidth(monName);
			monNameWidths.push_back(w);
			if (w>maxNameWidth)
				maxNameWidth = w;
		}
		int xOffset = maxNameWidth+5;
		float xScale = float(canvasWidth-xOffset-10)/totalCycles;
		if (xScale<2) xScale = 2;
		if (xScale>20) xScale = 20;
		for (mon=0; mon<monCount; mon++)
		{
			// Draw the signal trace
			glColor3f(0.0, 0.8, 0.0);
			DrawSignalTrace(xOffset+5, canvasHeight-spacing/2-mon*spacing, xScale, height, spacing*0.075, mon, totalCycles);

			// Draw the monitor name
			glColor3f(0.0, 0.0, 1.0);
			DrawText(xOffset-monNameWidths[mon], canvasHeight-spacing/2-mon*spacing-12/2, monNames[mon]);
		}
	}
	else if (mmz->moncount()==0)
	{
		glColor3f(0.5, 0.0, 0.0);
		DrawText(5, 10, wxT("No monitors"));
	}
	else
	{
		glColor3f(0.8, 0.0, 0.0);
		DrawText(5, 10, wxT("No simulation results to display"));
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
  canvas = new MyGLCanvas(this, wxID_ANY, monitor_mod, names_mod);
  leftsizer->Add(canvas, 3, wxEXPAND | wxALL, 10);
  outputTextCtrl = new wxTextCtrl(this, OUTPUT_TEXTCTRL_ID, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
  outputTextRedirect = new wxStreamToTextRedirector(outputTextCtrl);// Redirect all text sent to cout to the outputTextCtrl textbox
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
	wxFileDialog openFileDialog(this, wxT("Open logic circuit"), wxT(""), wxT(""), wxT("*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
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
	cout << "Loading file " << filename << endl;

	clearCircuit();
	scanner *smz = new scanner(nmz, filename);
	parser *pmz = new parser(netz, dmz, mmz, smz);
	bool result = pmz->readin();

	//TODO: maybe display a messagebox here or disable a few UI controls (like the Run button) if reading failed
	if (!result)
		cout << "Failed to load file" << endl;

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
