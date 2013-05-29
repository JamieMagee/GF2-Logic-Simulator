#ifndef gui_devices_h
#define gui_devices_h

#include "gui-devices-infopanels.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/combobox.h>
#include "network.h"
#include "circuit.h"
#include "observer.h"
#include "gui-misc.h"
#include <string>
#include <vector>
using namespace std;


class NewDeviceDialog: public wxDialog
{
public:
	NewDeviceDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title);
	devlink newdev;
private:
	circuit* c;
	DevicekindDropdown* dkindDropdown;
	DeviceNameTextCtrl* devicenameCtrl;
	void OnOK(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

class ChooseOutputDialog: public wxDialog
{
public:
	ChooseOutputDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title, const wxString& description);
	CircuitElementInfo result;
private:
	circuit* c;
	wxListBox* lbox;
	CircuitElementInfoVector outputs;
	void OnOK(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

// For a given output, allow input(s) to be chosen to connect to it
// Makes the connection(s) when the OK button is clicked
class ConnectToOutputDialog: public wxDialog
{
public:
	ConnectToOutputDialog(circuit* circ, outplink outp, wxWindow* parent, wxWindowID id, const wxString& title);
private:
	circuit* c;
	outplink o;
	CircuitElementInfoVector inputs;
	wxListBox* lbox;
	void OnOK(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

class DevicesDialog: public wxDialog
{
public:
	DevicesDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title, devlink d=NULL);
	~DevicesDialog();
	void OnDeviceSelectionChanged();
private:
	circuit* c;
	SelectedDevice* selectedDev;
	DevicesListBox* devListBox;
	DeviceDetailsPanel* detailsPanel;
	DeviceInputsPanel* inputsPanel;
	vector<DeviceOutputPanel*> outputPanels;
	wxBoxSizer* mainSizer;
	wxBoxSizer* outputsSizer;
	void DestroyDeviceWidgets();
	void OnDeleteButton(wxCommandEvent& event);
	void OnCreateButton(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};



#endif /* gui_devices_h */
