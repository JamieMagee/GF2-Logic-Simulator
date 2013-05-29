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
	DEVICECREATE_BUTTON_ID,
	DEVICEDELETE_BUTTON_ID,
	DEVICES_ADDCONN_BUTTON_ID,
	DEVICES_DELCONN_BUTTON_ID,
	DEVICENAME_TEXTCTRL_ID,
	DEVICEOUTPUT_MONITOR_CB_ID
};

extern wxString devicekindstrings[baddevice];

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


class DevicekindDropdown: public wxComboBox
{
public:
	DevicekindDropdown(wxWindow* parent, wxWindowID id = wxID_ANY, vector<devicekind> filterDevicekinds = vector<devicekind>());
	devicekind GetDevicekind();
	void SetDevicekind(devicekind dk);
private:
	vector<devicekind> filter;
};

class DeviceNameTextCtrl: public wxTextCtrl
{
public:
	DeviceNameTextCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& value = wxT(""));
	bool CheckValid(circuit* c, bool errorDialog=true);
};

void ShowErrorMsgDialog(wxWindow* parent, wxString txt);


class NewDeviceDialog: public wxDialog
{
public:
	NewDeviceDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title);
	devlink newdev;
private:
	circuit* c;
	DevicekindDropdown* dkindDropdown;
	DeviceNameTextCtrl* devNameCtrl;
	void OnOK(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

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
	vector<DeviceInfo> devs;
	bool isSetting;
	void OnLBoxSelectionChanged(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
};

// List of all inputs on a device and which output each is connected to, with buttons to add/remove connections
// select multiple
class DeviceInputsPanel: public wxPanel
{
public:
	DeviceInputsPanel(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id = wxID_ANY);
	~DeviceInputsPanel();
	void ReleasePointers();
	void OnDeviceSelectionChanged();
private:
	SelectedDevice* selectedDev;
	circuit* c;
	vector<inplink> inps;
	wxListBox* lbox;
	wxButton *btnAddConn, *btnDelConn;
	void UpdateInps();
	void OnConnectButton(wxCommandEvent& event);
	void OnDisconnectButton(wxCommandEvent& event);
	void OnLBoxSelectionChanged(wxCommandEvent& event);
	void UpdateControlStates();
	DECLARE_EVENT_TABLE()
};

// Lists the inputs of other devices that a particular device output is connected to, with buttons to add/remove connections
class DeviceOutputPanel: public wxPanel
{
public:
	DeviceOutputPanel(circuit* circ, outplink targetOutp, wxWindow* parent, wxWindowID id = wxID_ANY);
	~DeviceOutputPanel();
	void ReleasePointers();
	void OnDeviceSelectionChanged();
	void OnMonitorsChanged();
private:
	circuit* c;
	outplink outp;
	vector<CircuitElementInfo> inps;
	wxListBox* lbox;
	wxButton *btnAddConn, *btnDelConn;
	wxCheckBox* monitorCheckbox;
	void UpdateInps();
	void OnConnectButton(wxCommandEvent& event);
	void OnDisconnectButton(wxCommandEvent& event);
	void OnLBoxSelectionChanged(wxCommandEvent& event);
	void OnMonitorCheckboxChanged(wxCommandEvent& event);
	void UpdateControlStates();
	DECLARE_EVENT_TABLE()
};

// For devicekinds with nothing configurable: xorgate, dtype
class DeviceDetailsPanel: public wxPanel
{
public:
	DeviceDetailsPanel(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
	virtual void UpdateApplyButtonState_ExtraFields() {};
	virtual void OnApply_ExtraFields(bool& changedSomething) {};
	SelectedDevice* selectedDev;
	circuit* c;
	wxStaticBoxSizer* mainSizer;
	wxGridBagSizer* gridsizer;
	wxTextCtrl* devicenameCtrl;
	wxButton* updateBtn;
	wxStaticText* devicekindStaticText;

	wxSpinCtrl* spinCtrl;
	DevicekindDropdown* gateTypeDropdown;

	void UpdateApplyButtonState();
	void OnInputChanged(wxCommandEvent& event);
	void OnApply(wxCommandEvent& event);
	void ShowErrorMsg(wxString txt);
	DECLARE_EVENT_TABLE()
};

// For devicekinds: andgate, nandgate, orgate, norgate
class DeviceDetailsPanel_Gate: public DeviceDetailsPanel
{
public:
	DeviceDetailsPanel_Gate(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
	virtual void UpdateApplyButtonState_ExtraFields();
	virtual void OnApply_ExtraFields(bool& changedSomething);
};

// Could add one for devicekind=aswitch, but switch states are pretty easy to change anyway

// For devicekind: aclock
class DeviceDetailsPanel_Clock: public DeviceDetailsPanel
{
public:
	DeviceDetailsPanel_Clock(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id = wxID_ANY);
protected:
	virtual void UpdateApplyButtonState_ExtraFields();
	virtual void OnApply_ExtraFields(bool& changedSomething);
};


class ChooseOutputDialog: public wxDialog
{
public:
	ChooseOutputDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title, const wxString& description);
	CircuitElementInfo result;
private:
	circuit* c;
	wxListBox* lbox;
	vector<CircuitElementInfo> outputs;
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
	vector<CircuitElementInfo> inputs;
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
