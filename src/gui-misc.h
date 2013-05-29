#ifndef gui_misc_h
#define gui_misc_h

#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include "circuit.h"

// This file contains a few classes derived from wxWidgets classes, that have some extra functions to make them more convenient to use for logic simulator related tasks
// Also some functions for things done in lots of places in the GUI, like CircuitElementInfoVector_to_wxArrayString

extern wxString devicekindstrings[baddevice];

// Dropdown box to select a devicekind, optionally filtering the displayed types to those given in filterDevicekinds
class DevicekindDropdown: public wxComboBox
{
public:
	DevicekindDropdown(wxWindow* parent, wxWindowID id = wxID_ANY, vector<devicekind> filterDevicekinds = vector<devicekind>());
	devicekind GetDevicekind();
	void SetDevicekind(devicekind dk);
private:
	vector<devicekind> filter;
};

// Text control for entering a device name, with a function for validating the new name
class DeviceNameTextCtrl: public wxTextCtrl
{
public:
	DeviceNameTextCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& value = wxT(""));
	// Validate the new name, optionally showing an error message
	// allowExisting is a name that can be chosen even if a device already exists with the same name (so this control can be used for editing names of existing devices), use blankname to disallow any existing device name
	bool CheckValid(circuit* c,  name allowExisting=blankname, bool errorDialog=true);
};

// List of devices (single selection, selection changes are handled by the supplied SelectedDevice object)
class DevicesListBox: public wxListBox
{
public:
	DevicesListBox(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id);
	~DevicesListBox();
	void OnCircuitChanged();
	void OnDeviceSelectionChanged();
	void ReleasePointers();
	devlink GetSelectionAfterDelete();
private:
	SelectedDevice* selectedDev;
	circuit* c;
	CircuitElementInfoVector devs;
	bool isSetting;
	void OnLBoxSelectionChanged(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

// Show an error messagebox
void ShowErrorMsgDialog(wxWindow* parent, wxString txt);

// Put namestrs from a CircuitElementInfoVector into a wxArrayString
void CircuitElementInfoVector_to_wxArrayString(const CircuitElementInfoVector& src, wxArrayString& dest);

#endif /* gui_misc_h */
