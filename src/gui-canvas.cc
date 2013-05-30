#include "gui-canvas.h"
#include <GL/glut.h>
#include <cmath>

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
