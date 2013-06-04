#ifndef gui_options_h
#define gui_options_h

#include "observer.h"
#include "circuit.h"
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>

const wxString configName = wxT("GF2Logsim-2013-8");

class LogsimOptions
{
public:
	LogsimOptions();
	~LogsimOptions();

	double xScaleMin, xScaleMax;
	int sigHeightMin, sigHeightMax;
	int continuousRate;
	bool debugMachineCycles, debugSim;
	ObserverSubject optionsChanged;

	void ResetOptions();
	void Load();
	void Save();
};

class OptionsDialog : public wxDialog
{
private:
	LogsimOptions *options;
	wxSpinCtrl* xScaleMinCtrl;
	wxSpinCtrl* xScaleMaxCtrl;
	wxSpinCtrl* sigHeightMinCtrl;
	wxSpinCtrl* sigHeightMaxCtrl;
	wxSpinCtrl* continuousRateCtrl;
	wxCheckBox* debugMachineCyclesCtrl;
	wxCheckBox* debugSimCtrl;
public:
	OptionsDialog(LogsimOptions* options_in, wxWindow* parent, wxWindowID id, const wxString& title);
	void OnOK(wxCommandEvent& event);
	void OnInputChanged(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

#endif  /* gui_options_h */
