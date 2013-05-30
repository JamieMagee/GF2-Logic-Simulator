#ifndef gui_h
#define gui_h

#include "gui-canvas.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
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
	void OnButtonEditDevs(wxCommandEvent& event);
	void UpdateControlStates();
	void OnMenuClearCircuit(wxCommandEvent &event);
 
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

#endif /* gui_h */
