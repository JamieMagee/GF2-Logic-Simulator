#include "gui.h"
#include "wx_icon.xpm"
#include <iostream>
#include <vector>
#include <algorithm>
#include "scanner.h"
#include "parser.h"
#include "gui-devices.h"
#include "gui-id.h"

using namespace std;


// MyFrame ///////////////////////////////////////////////////////////////////////////////////////


BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_MENU(wxID_EXIT, MyFrame::OnExit)
  EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
  EVT_MENU(MENU_CLEAR_CIRCUIT, MyFrame::OnMenuClearCircuit)
  EVT_MENU(MENU_RELOAD_FILE, MyFrame::OnFileReload)
  EVT_MENU(wxID_OPEN, MyFrame::OnOpenFile)
  EVT_BUTTON(SIMCTRL_BUTTON_RUN_ID, MyFrame::OnButtonRun)
  EVT_BUTTON(SIMCTRL_BUTTON_CONT_ID, MyFrame::OnButtonContinue)
  EVT_BUTTON(SIMCTRL_BUTTON_RUNCONT_ID, MyFrame::OnButtonRunContinuously)
  EVT_BUTTON(MONITORS_ADD_BUTTON_ID, MyFrame::OnButtonAddMon)
  EVT_BUTTON(MONITORS_DEL_BUTTON_ID, MyFrame::OnButtonDelMon)
  EVT_BUTTON(DEVICES_EDIT_BUTTON_ID, MyFrame::OnButtonEditDevs)
  EVT_TIMER(RUNSIM_TIMER_ID, MyFrame::OnRunSimTimer)
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
	filedlgName = _("");
	filedlgDir = _("../examples/");
	options = new LogsimOptions();
	runsimTimer.SetOwner(this, RUNSIM_TIMER_ID);

	// Menu items
	fileMenu = new wxMenu;
	fileMenu->Append(wxID_OPEN);
	fileMenu->Append(MENU_RELOAD_FILE, _("Reload\tCtrl+R"));
	fileMenu->Enable(MENU_RELOAD_FILE, false);
	fileMenu->Append(MENU_CLEAR_CIRCUIT, _("Clear circuit"));
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
	canvas = new MyGLCanvas(c, options, mainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
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
	simctrls_button_sizer->Add(simctrl_run, 0, wxALL & ~wxBOTTOM, 10);
	simctrls_button_sizer->Add(simctrl_continue, 0, wxALL & ~wxBOTTOM, 10);
	// Simulation cycle number spinner
	simctrls_cycles_sizer->Add(new wxStaticText(simctrls_container, wxID_ANY, _("Cycles")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 10);
	spin = new wxSpinCtrl(simctrls_container, MY_SPINCNTRL_ID, wxString(wxT("42")));
	spin->SetRange(1,1000000);
	simctrls_cycles_sizer->Add(spin, 0, wxALL, 10);
	// Run continuously button
	simctrl_runcontinuous = new wxButton(simctrls_container, SIMCTRL_BUTTON_RUNCONT_ID, _("Run continuously"));

	simctrls_sizer->Add(simctrls_cycles_sizer);
	simctrls_sizer->Add(simctrls_button_sizer);
	simctrls_sizer->Add(simctrl_runcontinuous, 0, wxALL | wxEXPAND, 10);
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

void MyFrame::OnMenuClearCircuit(wxCommandEvent &event)
{
	c->Clear();
}

void MyFrame::OnFileReload(wxCommandEvent &event)
{
	loadFile(lastFilePath.c_str());
}

void MyFrame::OnOpenFile(wxCommandEvent &event)
  // Callback for the File -> Open menu item
{
	wxFileDialog openFileDialog(this, _("Open logic circuit"), filedlgDir, filedlgName, _("Logic circuit files (*.gf2)|*.gf2|All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_CHANGE_DIR);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return; // cancelled, don't open a file
	filedlgDir = openFileDialog.GetDirectory();
	filedlgName = openFileDialog.GetFilename();
	loadFile(openFileDialog.GetPath().mb_str());
}

bool MyFrame::loadFile(const char * filename)
// load a file (can be called by menu File->Open or for the command line argument)
{
	bool result; //True if file is opened correctly

	fileMenu->Enable(MENU_RELOAD_FILE, true);
	lastFilePath = filename;

	// Clear log window
	outputTextCtrl->ChangeValue(wxT(""));
	cout << "Loading file " << filename << endl;

	c->Clear();

	scanner *smz = new scanner(c->nmz(), filename, result);
	error *erz = new error(smz);
	parser *pmz = new parser(c->netz(), c->dmz(), c->mmz(), smz, erz);
	if (result) result = pmz->readin();

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
	delete erz;
	delete smz;

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
		{
			simctrl_continue->Disable();
			if (c->GetTotalCycles() && runsimTimer.IsRunning())// some but not all empty, and continuously scrolling
				SetContinuousRun(false);
		}
		else
			simctrl_continue->Enable();
		
	}
}

void MyFrame::OnButtonRun(wxCommandEvent &event)
  // Callback for the run simulation button
{
	c->Simulate(spin->GetValue(), true, options->debugMachineCycles);
	simctrl_continue->Enable();
}

void MyFrame::OnButtonContinue(wxCommandEvent &event)
  // Callback for the continue simulation button
{
	c->Simulate(spin->GetValue(), false, options->debugMachineCycles);
}

void MyFrame::OnButtonRunContinuously(wxCommandEvent& event)
{
	SetContinuousRun(!runsimTimer.IsRunning());
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

void MyFrame::SetContinuousRun(bool state)
{
	if (state == runsimTimer.IsRunning()) return;

	if (state)
	{
		bool someEmpty = (c->GetTotalCycles()==0);
		for (int i=0; i<c->mmz()->moncount(); i++)
		{
			if (c->mmz()->getsamplecount(i)==0)
				someEmpty = true;
		}
		if (someEmpty) c->ResetMonitors();
		if (!runsimTimer.Start(10))
		{
			ShowErrorMsgDialog(this, _("Could not start timer"));
			return;
		}
		simctrl_runcontinuous->SetLabel(_("Stop"));
	}
	else
	{
		runsimTimer.Stop();
		simctrl_runcontinuous->SetLabel(_("Run continuously"));
	}
}

void MyFrame::OnRunSimTimer(wxTimerEvent& event)
{
	if (!c->Simulate(1, false, options->debugMachineCycles))
		SetContinuousRun(false);
}

AddMonitorsDialog::AddMonitorsDialog(circuit* circ, wxWindow* parent, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
	wxDialog(parent, wxID_ANY, title, pos, size, style)
{
	SetIcon(wxIcon(wx_icon));

	c = circ;
	oldMonCount = c->mmz()->moncount();

	// Make a list of unmonitored outputs
	c->GetUnmonitoredOutputs(&availableOutputs);
	sort(availableOutputs.begin(), availableOutputs.end(), CircuitElementInfo_namestrcmp);
	wxArrayString displayedOutputs;
	CircuitElementInfoVector_to_wxArrayString(availableOutputs, displayedOutputs);

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
		c->mmz()->makemonitor(availableOutputs[selections[i]].d->id, availableOutputs[selections[i]].o->id, ok);
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
