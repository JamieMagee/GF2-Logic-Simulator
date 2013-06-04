#ifndef gui_options_h
#define gui_options_h

#include "observer.h"
#include "circuit.h"
#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <wx/checkbox.h>

class LogsimOptions
{
public:
	LogsimOptions();

	double xScaleMin, xScaleMax;
	int spacingMin, spacingMax;
	int continuousRate;
	bool debugMachineCycles, debugSim;
	ObserverSubject optionsChanged;

	void ResetOptions();
};

class OptionsDialog : public wxDialog
{
private:
	LogsimOptions *options;
	wxSpinCtrl* xScaleMinCtrl;
	wxSpinCtrl* xScaleMaxCtrl;
	wxSpinCtrl* spacingMinCtrl;
	wxSpinCtrl* spacingMaxCtrl;
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
