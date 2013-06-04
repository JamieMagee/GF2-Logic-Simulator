#ifndef gui_options_h
#define gui_options_h

#include "observer.h"
#include <wx/wx.h>
#include <wx/dialog.h>

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
public:
	OptionsDialog(LogsimOptions* options, wxWindow* parent, wxWindowID id, const wxString& title);
};

#endif  /* gui_options_h */
