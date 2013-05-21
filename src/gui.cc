#include "gui.h"
#include <GL/glut.h>
#include "wx_icon.xpm"
#include <iostream>
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
  cyclesdisplayed = -1;
}

void MyGLCanvas::DrawSignalTrace(int xOffset, int yOffset, int xScale, int height, int mon, int cycles)
{
	glBegin(GL_LINE_STRIP);
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

void MyGLCanvas::Render(wxString example_text, int cycles)
  // Draws canvas contents - the following example writes the string "example text" onto the canvas
  // and draws a signal trace. The trace is artificial if the simulator has not yet been run.
  // When the simulator is run, the number of cycles is passed as a parameter and the first monitor
  // trace is displayed.
{
  float y;
  unsigned int i;
  asignal s;

  if (cycles >= 0) cyclesdisplayed = cycles;

  SetCurrent();
  if (!init) {
    InitGL();
    init = true;
  }
  glClear(GL_COLOR_BUFFER_BIT);

	if ((cyclesdisplayed >= 0) && (mmz->moncount() > 0))
	{
		int monCount = mmz->moncount();
		int mon, height=20, xScale=5, spacing=30;
		name dev, outp;
		for (mon=0; mon<monCount; mon++)
		{
			// Draw the signal trace
			glColor3f(0.0, 0.8, 0.0);
			DrawSignalTrace(30, 10+spacing/2+mon*spacing, xScale, height, mon, cyclesdisplayed);

			// Draw the monitor name
			mmz->getmonname(mon, dev, outp);
			wxString monName(nmz->getnamestring(dev).c_str(), wxConvUTF8);
			if (outp!=blankname)
				monName += wxT(".") + wxString(nmz->getnamestring(outp).c_str(), wxConvUTF8);
			glColor3f(0.0, 0.0, 1.0);
			DrawText(1, 10+spacing/2+mon*spacing-12/2, monName);
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

  // Example of how to use GLUT to draw text on the canvas
  glColor3f(0.0, 0.0, 1.0);
  glRasterPos2f(10, 100);
  for (i = 0; i < example_text.Len(); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, example_text[i]);

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
  EVT_BUTTON(MY_BUTTON_ID, MyFrame::OnButton)
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
  topsizer->Add(leftsizer, 1, wxEXPAND | wxALL, 10);

  wxBoxSizer *button_sizer = new wxBoxSizer(wxVERTICAL);
  button_sizer->Add(new wxButton(this, MY_BUTTON_ID, wxT("Run")), 0, wxALL, 10);
  button_sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Cycles")), 0, wxTOP|wxLEFT|wxRIGHT, 10);
  spin = new wxSpinCtrl(this, MY_SPINCNTRL_ID, wxString(wxT("31")));
  button_sizer->Add(spin, 0 , wxALL, 10);

  button_sizer->Add(new wxTextCtrl(this, MY_TEXTCTRL_ID, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER), 0 , wxALL, 10);
  topsizer->Add(button_sizer, 0, wxALIGN_CENTER);

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

void MyFrame::OnButton(wxCommandEvent &event)
  // Callback for the push button
{
  int n, ncycles;

  cyclescompleted = 0;
  mmz->resetmonitor ();
  runnetwork(spin->GetValue());
  canvas->Render(wxT("Run button pressed"), cyclescompleted);
  cout << "Run button pressed" << endl;// Testing cout redirection
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
  // Function to run the network, derived from corresponding function in userint.cc
{
  bool ok = true;
  int n = ncycles;

  while ((n > 0) && ok) {
    dmz->executedevices (ok);
    if (ok) {
      n--;
      mmz->recordsignals ();
    } else
      cout << "Error: network is oscillating" << endl;
  }
  if (ok) cyclescompleted = cyclescompleted + ncycles;
  else cyclescompleted = 0;
}
