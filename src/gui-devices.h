#ifndef gui_devices_h
#define gui_devices_h

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/combobox.h>
#include <wx/gbsizer.h>
#include "network.h"
#include "circuit.h"
#include "observer.h"
#include <string>
#include <vector>
using namespace std;

enum
{
	DEVICES_APPLY_BUTTON_ID = wxID_HIGHEST + 1,
	DEVICES_DELETE_BUTTON_ID,
	DEVICES_ADDCONN_BUTTON_ID,
	DEVICES_DELCONN_BUTTON_ID,
	DEVICENAME_TEXTCTRL_ID
};

extern wxString devicenamestrings[baddevice];

class SelectedDevice
{
private:
	devlink selected;
public:
	ObserverSubject changed;
	devlink Get()
	{
		return selected;
	}
	void Set(devlink d)
	{
		if (d!=selected)
		{
			selected = d;
			changed.Trigger();
		}
	}
};

struct DeviceInfo
{
	devlink d;
	string devname;
};

class DeviceKindComboBox: public wxComboBox
{
	;
};

class DevicesListBox: public wxListBox
{
public:
	DevicesListBox(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id);
	~DevicesListBox();
	void OnCircuitChanged();
	void OnDeviceSelectionChanged();
	void ReleasePointers();
private:
	SelectedDevice* selectedDev;
	circuit* c;
	vector<DeviceInfo> devs;
	bool isSetting;
	void OnLBoxSelectionChanged(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

// List of all inputs on a device and which output each is connected to, with buttons to add/remove connections
class DeviceInputsPanel: public wxPanel
{
	//circuit* circ, devlink
};

// Lists the inputs of other devices that a particular device output is connected to, with buttons to add/remove connections
class DeviceOutputPanel: public wxPanel
{
	//circuit* circ, devlink, outplink
};

// For devicekinds with nothing configurable: xorgate, dtype
class DeviceDetailsPanel: public wxPanel
{
public:
	DeviceDetailsPanel(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id = wxID_ANY);
private:
	virtual void CreateExtraFields() {};
	virtual void UpdateApplyButtonState_ExtraFields() {};
	virtual void OnApply_ExtraFields(bool& changedSomething) {};
	SelectedDevice* selectedDev;
	circuit* c;
	wxStaticBoxSizer* mainSizer;
	wxGridBagSizer* gridsizer;
	wxTextCtrl* devicenameCtrl;
	wxButton* updateBtn;
	void UpdateApplyButtonState();
	void OnInputChanged(wxCommandEvent& event);
	void OnApply(wxCommandEvent& event);
	void ShowErrorMsg(wxString txt);
	DECLARE_EVENT_TABLE()
};

// For devicekinds: andgate, nandgate, orgate, norgate
class DeviceDetailsPanel_Gate: public DeviceDetailsPanel
{
	;
};

// For devicekind: aswitch
class DeviceDetailsPanel_Switch: public DeviceDetailsPanel
{
	;
};

// For devicekind: aclock
class DeviceDetailsPanel_Clock: public DeviceDetailsPanel
{
	;
};

// For a given input, allow an output to be chosen to connect to it
class ConnectToInputDialog: public wxDialog
{
	;
};

// For a given output, allow input(s) to be chosen to connect to it
class ConnectToOutputDialog: public wxDialog
{
	;
};

class DevicesDialog: public wxDialog
{
public:
	DevicesDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title, devlink d=NULL);
	void OnDeviceSelectionChanged();
	~DevicesDialog();
private:
	circuit* c;
	SelectedDevice* selectedDev;
	DevicesListBox* devListBox;
	DeviceDetailsPanel* detailsPanel;
	DeviceInputsPanel* inputsPanel;
	vector<DeviceOutputPanel*> outputPanels;
	wxBoxSizer* mainSizer;

	void DestroyDeviceWidgets();
	DECLARE_EVENT_TABLE()
};



#endif /* gui_devices_h */
