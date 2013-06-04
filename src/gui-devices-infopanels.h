#ifndef gui_devices_infopanels_h
#define gui_devices_infopanels_h

#include "circuit.h"
#include "observer.h"
#include "gui-misc.h"
#include <wx/panel.h>
#include <wx/gbsizer.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/button.h>

// Panel for display and modification of device properties
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
	DeviceNameTextCtrl* devicenameCtrl;
	wxButton* updateBtn;
	wxStaticText* devicekindStaticText;

	wxSpinCtrl* spinCtrl;
	wxTextCtrl* textCtrl;
	DevicekindDropdown* gateTypeDropdown;
	string waveformText;

	void UpdateApplyButtonState();
	void OnInputChanged(wxCommandEvent& event);
	void OnApply(wxCommandEvent& event);
	void ShowErrorMsg(wxString txt);
	DECLARE_EVENT_TABLE()
};

// List of all inputs on a device and which output each is connected to, with buttons to add/remove connections
class DeviceInputsPanel: public wxPanel
{
public:
	DeviceInputsPanel(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id = wxID_ANY);
	~DeviceInputsPanel();
	void ReleasePointers();
private:
	SelectedDevice* selectedDev;
	circuit* c;
	CircuitElementInfoVector inps;
	wxListBox* lbox;
	wxButton *btnAddConn, *btnDelConn;
	void UpdateInps();
	void OnConnectButton(wxCommandEvent& event);
	void OnDisconnectButton(wxCommandEvent& event);
	void OnLBoxSelectionChanged(wxCommandEvent& event);
	void UpdateControlStates();
	void OnDeviceSelectionChanged();
	DECLARE_EVENT_TABLE()
};

// Lists the inputs of other devices that a particular device output is connected to, with buttons to add/remove connections
class DeviceOutputPanel: public wxPanel
{
public:
	DeviceOutputPanel(circuit* circ, outplink targetOutp, wxWindow* parent, wxWindowID id = wxID_ANY);
	~DeviceOutputPanel();
	void ReleasePointers();
private:
	circuit* c;
	outplink outp;
	CircuitElementInfoVector inps;
	wxListBox* lbox;
	wxButton *btnAddConn, *btnDelConn;
	wxCheckBox* monitorCheckbox;
	void UpdateInps();
	void OnConnectButton(wxCommandEvent& event);
	void OnDisconnectButton(wxCommandEvent& event);
	void OnLBoxSelectionChanged(wxCommandEvent& event);
	void OnMonitorCheckboxChanged(wxCommandEvent& event);
	void UpdateControlStates();
	void OnDeviceSelectionChanged();
	void OnMonitorsChanged();
	DECLARE_EVENT_TABLE()
};

#endif /* gui_devices_infopanels_h */
