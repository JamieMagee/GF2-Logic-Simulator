#include "gui-devices-infopanels.h"
#include "gui-devices.h"
#include "gui-id.h"
#include <algorithm>

using namespace std;

DeviceInputsPanel::DeviceInputsPanel(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id) :
	wxPanel(parent, id)
{
	selectedDev = selectedDev_in;
	c = circ;

	wxStaticBoxSizer* mainSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Inputs"));
	// Listbox to show all inputs for this device
	lbox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_NEEDED_SB | wxLB_EXTENDED);
	lbox->SetMinSize(wxSize(150,100));
	mainSizer->Add(lbox, 1, wxEXPAND | (wxALL & ~wxBOTTOM), 10);
	// Connect and disconnect buttons
	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	btnAddConn = new wxButton(this, DEVICES_ADDCONN_BUTTON_ID, _("Connect"));
	btnDelConn = new wxButton(this, DEVICES_DELCONN_BUTTON_ID, _("Disconnect"));
	buttonsSizer->Add(btnAddConn, 1, wxEXPAND, 10);
	buttonsSizer->Add(btnDelConn, 1, wxEXPAND, 10);
	mainSizer->Add(buttonsSizer, 0, wxEXPAND | (wxALL & ~wxTOP) , 10);
	SetSizerAndFit(mainSizer);

	// Listen for device selection changes, and changes to device details
	selectedDev->changed.Attach(this, &DeviceInputsPanel::OnDeviceSelectionChanged);
	c->circuitChanged.Attach(this, &DeviceInputsPanel::UpdateInps);

	UpdateInps();
	UpdateControlStates();
}

DeviceInputsPanel::~DeviceInputsPanel()
{
	ReleasePointers();
}

void DeviceInputsPanel::ReleasePointers()
{
	if (selectedDev)
		selectedDev->changed.Detach(this);
	selectedDev = NULL;
	if (c)
		c->circuitChanged.Detach(this);
	c = NULL;
}

void DeviceInputsPanel::UpdateInps()
{
	wxArrayInt selections;
	lbox->GetSelections(selections);

	inps.resize(0);
	wxArrayString lboxData;
	if (selectedDev && c && selectedDev->Get())
	{
		// Make a list of all inputs for the selected device
		inps.push_back_dev_inputs(selectedDev->Get());
		// Reverse it, since creating from I1..I16 means ilist points to the last one, I16
		reverse(inps.begin(), inps.end());
		// Put input names in the listbox, with a description of what (if anything) is connected to each one
		lboxData.Alloc(inps.size());
		for (CircuitElementInfoVector::iterator it=inps.begin(); it<inps.end(); ++it)
		{
			inplink i = it->i;
			wxString desc(c->nmz()->getnamestring(i->id).c_str(), wxConvUTF8);
			if (i->connect)
			{
				desc = desc + wxT(" (") + _("connected to ") + wxString(c->netz()->getsignalstring(i->connect->dev->id, i->connect->id).c_str(), wxConvUTF8) + wxT(")");
			}
			else
			{
				desc = desc + _(" (unconnected)");
			}
			lboxData.Add(desc);
		}
	}
	lbox->Set(lboxData);

	UpdateControlStates();
}

void DeviceInputsPanel::OnDeviceSelectionChanged()
{
	lbox->SetSelection(wxNOT_FOUND);
	if (c && selectedDev && selectedDev->Get())
	{
		// Selected device changed, update the displayed inputs
		UpdateInps();
		Show();
	}
	else
	{
		// Hide the device inputs details panel if no device is selected
		Hide();
	}
}

void DeviceInputsPanel::OnLBoxSelectionChanged(wxCommandEvent& event)
{
	UpdateControlStates();
}

void DeviceInputsPanel::UpdateControlStates()
{
	// Enable or disable add+remove connection buttons depending on whether any inputs are selected
	wxArrayInt selections;
	lbox->GetSelections(selections);
	if (selections.GetCount())
	{
		btnAddConn->Enable();
		btnDelConn->Enable();
	}
	else
	{
		btnAddConn->Disable();
		btnDelConn->Disable();
	}
}

void DeviceInputsPanel::OnConnectButton(wxCommandEvent& event)
{
	wxArrayInt selections;
	lbox->GetSelections(selections);
	ChooseOutputDialog dlg(c, this, wxID_ANY, _("Choose output"), wxPLURAL("Choose an output to connect this input to:", "Choose an output to connect these inputs to:", selections.GetCount()));
	if (dlg.ShowModal()==wxID_OK && dlg.result.o)
	{
		wxArrayInt selections;
		lbox->GetSelections(selections);
		// Connect all selected inputs to the output chosen in the dialog
		for (int i=0; i<selections.GetCount(); i++)
		{
			if (selections[i]<inps.size())
			{
				inps[selections[i]].i->connect = dlg.result.o;
			}
		}
		c->circuitChanged.Trigger();
	}
}

void DeviceInputsPanel::OnDisconnectButton(wxCommandEvent& event)
{
	wxArrayInt selections;
	lbox->GetSelections(selections);
	// Disconnect all selected inputs from whatever they are connected to
	for (int i=0; i<selections.GetCount(); i++)
	{
		if (i<inps.size()) inps[selections[i]].i->connect = NULL;
	}
	c->circuitChanged.Trigger();
}

BEGIN_EVENT_TABLE(DeviceInputsPanel, wxPanel)
	EVT_LISTBOX(wxID_ANY, DeviceInputsPanel::OnLBoxSelectionChanged)
	EVT_BUTTON(DEVICES_ADDCONN_BUTTON_ID, DeviceInputsPanel::OnConnectButton)
	EVT_BUTTON(DEVICES_DELCONN_BUTTON_ID, DeviceInputsPanel::OnDisconnectButton)
END_EVENT_TABLE()


DeviceOutputPanel::DeviceOutputPanel(circuit* circ, outplink targetOutp, wxWindow* parent, wxWindowID id) :
	wxPanel(parent, id)
{
	outp = targetOutp;
	c = circ;

	wxString title = _("Output");
	if (outp->id != blankname)
		title = title + wxT(" ") + wxString(c->nmz()->getnamestring(outp->id).c_str(),wxConvUTF8);
	wxStaticBoxSizer* mainSizer = new wxStaticBoxSizer(wxVERTICAL, this, title);
	// Checkbox to add/remove a monitor on this output
	monitorCheckbox = new wxCheckBox(this, DEVICEOUTPUT_MONITOR_CB_ID, _("Monitored"));
	mainSizer->Add(monitorCheckbox, 0, wxALL, 10);
	// Listbox showing all connected inputs
	mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Inputs connected to this output:")), 0, wxLEFT | wxRIGHT, 10);
	lbox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_NEEDED_SB | wxLB_EXTENDED);
	lbox->SetMinSize(wxSize(150,50));
	mainSizer->Add(lbox, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
	// Connect and disconnect buttons
	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	btnAddConn = new wxButton(this, DEVICES_ADDCONN_BUTTON_ID, _("Connect"));
	btnDelConn = new wxButton(this, DEVICES_DELCONN_BUTTON_ID, _("Disconnect"));
	buttonsSizer->Add(btnAddConn, 1, wxEXPAND, 10);
	buttonsSizer->Add(btnDelConn, 1, wxEXPAND, 10);
	mainSizer->Add(buttonsSizer, 0, wxEXPAND | (wxALL & ~wxTOP) , 10);
	SetSizerAndFit(mainSizer);

	// Listen for changes to device details and monitors
	c->circuitChanged.Attach(this, &DeviceOutputPanel::UpdateInps);
	c->monitorsChanged.Attach(this, &DeviceOutputPanel::OnMonitorsChanged);

	UpdateInps();
	UpdateControlStates();
	OnMonitorsChanged();
}

DeviceOutputPanel::~DeviceOutputPanel()
{
	ReleasePointers();
}

void DeviceOutputPanel::ReleasePointers()
{
	if (c)
	{
		c->circuitChanged.Detach(this);
		c->monitorsChanged.Detach(this);
	}
	c = NULL;
}

void DeviceOutputPanel::OnMonitorsChanged()
{
	// If monitors are added or removed, update the value of the checkbox indicating whether this output is monitored
	monitorCheckbox->SetValue(c->mmz()->IsMonitored(outp));
}

void DeviceOutputPanel::OnMonitorCheckboxChanged(wxCommandEvent& event)
{
	bool ok = false;
	// Monitor checkbox changed, add or remove a monitor for this device
	if (monitorCheckbox->IsChecked())
	{
		c->mmz()->makemonitor(outp->dev->id, outp->id, ok);
		if (c->GetTotalCycles()!=0)
			cout << wxString(_("Monitor added, run simulation again to see updated signals")).mb_str() << endl;
	}
	else
	{
		c->mmz()->remmonitor(outp->dev->id, outp->id, ok);
	}
	if (ok) c->monitorsChanged.Trigger();
}

void DeviceOutputPanel::UpdateInps()
{
	wxArrayInt selections;
	lbox->GetSelections(selections);
	inps.resize(0);
	wxArrayString lboxData;
	if (c)
	{
		// Make a list of all inputs connected to this output
		devlink d = c->netz()->devicelist();
		while (d != NULL)
		{
			inplink i = d->ilist;
			while (i != NULL)
			{
				if (i->connect == outp) inps.push_back(CircuitElementInfo(d,i));
				i = i->next;
			}
			d = d->next;
		}
		inps.UpdateSignalNames(c);
		sort(inps.begin(), inps.end(), CircuitElementInfo_namestrcmp);
		// Put the names in the listbox
		lboxData.Alloc(inps.size());
		for (CircuitElementInfoVector::iterator it=inps.begin(); it<inps.end(); ++it)
		{
			lboxData.Add(wxString(it->namestr.c_str(), wxConvUTF8));
		}
	}
	lbox->Set(lboxData);
	UpdateControlStates();
}

void DeviceOutputPanel::OnLBoxSelectionChanged(wxCommandEvent& event)
{
	UpdateControlStates();
}

void DeviceOutputPanel::UpdateControlStates()
{
	// Enable or disable "remove connection" button depending on whether any inputs are currently selected
	wxArrayInt selections;
	lbox->GetSelections(selections);
	if (selections.GetCount())
	{
		btnDelConn->Enable();
	}
	else
	{
		btnDelConn->Disable();
	}
}

void DeviceOutputPanel::OnConnectButton(wxCommandEvent& event)
{
	// Dialog to choose some inputs to connect to this output
	ConnectToOutputDialog dlg(c, outp, this, wxID_ANY, _("Choose inputs"));
	dlg.ShowModal();
}

void DeviceOutputPanel::OnDisconnectButton(wxCommandEvent& event)
{
	// Disconnect all selected inputs
	wxArrayInt selections;
	lbox->GetSelections(selections);
	for (int i=0; i<selections.GetCount(); i++)
	{
		if (i<inps.size()) inps[selections[i]].i->connect = NULL;
	}
	c->circuitChanged.Trigger();
}

BEGIN_EVENT_TABLE(DeviceOutputPanel, wxPanel)
	EVT_LISTBOX(wxID_ANY, DeviceOutputPanel::OnLBoxSelectionChanged)
	EVT_CHECKBOX(DEVICEOUTPUT_MONITOR_CB_ID, DeviceOutputPanel::OnMonitorCheckboxChanged)
	EVT_BUTTON(DEVICES_ADDCONN_BUTTON_ID, DeviceOutputPanel::OnConnectButton)
	EVT_BUTTON(DEVICES_DELCONN_BUTTON_ID, DeviceOutputPanel::OnDisconnectButton)
END_EVENT_TABLE()


DeviceDetailsPanel::DeviceDetailsPanel(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id) :
	wxPanel(parent, id)
{
	selectedDev = selectedDev_in;
	c = circ;

	mainSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Device details"));
	devicekind dk = baddevice;
	devlink d = selectedDev->Get();
	if (d)
	{
		dk = d->kind;

		wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
		gridsizer = new wxGridBagSizer();
		wxString kindtext = wxT("Unknown device");
		if (dk>=0 && dk<baddevice)
			kindtext = devicekindstrings[dk];

		// Device type text
		gridsizer->Add(new wxStaticText(this, wxID_ANY, _("Device type:")), wxGBPosition(0,0), wxDefaultSpan, (wxALL & ~wxRIGHT) | wxALIGN_CENTER_VERTICAL, 10);
		devicekindStaticText = new wxStaticText(this, wxID_ANY, kindtext);
		gridsizer->Add(devicekindStaticText, wxGBPosition(0,1), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL, 10);
		// Device name textbox
		gridsizer->Add(new wxStaticText(this, wxID_ANY, _("Name:")), wxGBPosition(0,3), wxDefaultSpan, (wxALL & ~wxRIGHT) | wxALIGN_CENTER_VERTICAL, 10);
		devicenameCtrl = new DeviceNameTextCtrl(this, DEVICENAME_TEXTCTRL_ID, wxString(c->nmz()->getnamestring(d->id).c_str(), wxConvUTF8));
		gridsizer->Add(devicenameCtrl, wxGBPosition(0,4), wxDefaultSpan, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 10);
		gridsizer->AddGrowableCol(1,3);
		gridsizer->AddGrowableCol(4,5);

		// Widgets specific to particular devices
		if (d->kind == aclock)
		{
			// Half time period ("frequency" in existing code)
			spinCtrl = new wxSpinCtrl(this);
			spinCtrl->SetValue(selectedDev->Get()->frequency);
			spinCtrl->SetRange(1,INT_MAX);
			gridsizer->Add(new wxStaticText(this, wxID_ANY, _("Half-time-period:")), wxGBPosition(1,3), wxDefaultSpan, wxLEFT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 10);
			gridsizer->Add(spinCtrl, wxGBPosition(1,4), wxDefaultSpan, (wxALL & ~wxTOP) | wxEXPAND | wxALIGN_CENTER_VERTICAL, 10);
		}
		else if (d->kind == andgate || d->kind == nandgate || d->kind == orgate || d->kind == norgate)
		{
			// Gate type dropdown (allow changes of device kind between and/nand/or/nor, since they have identical parameters and input/output rules)
			vector<devicekind> gateTypeChoices;
			gateTypeChoices.push_back(andgate);
			gateTypeChoices.push_back(nandgate);
			gateTypeChoices.push_back(orgate);
			gateTypeChoices.push_back(norgate);
			gateTypeDropdown = new DevicekindDropdown(this, wxID_ANY, gateTypeChoices);
			gateTypeDropdown->SetDevicekind(d->kind);
			gridsizer->Add(new wxStaticText(this, wxID_ANY, _("Gate type:")), wxGBPosition(1,0), wxDefaultSpan, wxLEFT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 10);
			gridsizer->Add(gateTypeDropdown, wxGBPosition(1,1), wxDefaultSpan, (wxALL & ~wxTOP) | wxEXPAND | wxALIGN_CENTER_VERTICAL, 10);
			// Spinner to select number of inputs
			spinCtrl = new wxSpinCtrl(this);
			spinCtrl->SetValue(GetLinkedListLength(selectedDev->Get()->ilist));
			spinCtrl->SetRange(1,16);
			gridsizer->Add(new wxStaticText(this, wxID_ANY, _("Inputs:")), wxGBPosition(1,3), wxDefaultSpan, wxLEFT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 10);
			gridsizer->Add(spinCtrl, wxGBPosition(1,4), wxDefaultSpan, (wxALL & ~wxTOP) | wxEXPAND | wxALIGN_CENTER_VERTICAL, 10);
		}
		mainSizer->Add(gridsizer, 0, wxEXPAND);

		// Delete device button, and button for applying changes to device details
		buttonsSizer->AddStretchSpacer();
		buttonsSizer->Add(new wxButton(this, DEVICEDELETE_BUTTON_ID, _("Delete device")), 0, (wxALL & ~wxLEFT) | wxEXPAND, 10);
		updateBtn = new wxButton(this, DEVICES_APPLY_BUTTON_ID, _("Apply changes"));
		updateBtn->Disable();
		buttonsSizer->Add(updateBtn, 0, (wxALL & ~wxLEFT) | wxEXPAND, 10);
		mainSizer->Add(buttonsSizer, 0, wxEXPAND);
	}
	else
	{
		mainSizer->Add(new wxStaticText(this, wxID_ANY, _("No device selected")));
	}

	SetSizerAndFit(mainSizer);
}

void DeviceDetailsPanel::OnApply(wxCommandEvent& event)
{
	if (!c || !selectedDev || !selectedDev->Get()) return;

	devlink d = selectedDev->Get();
	bool changedSomething = false;

	if (wxString(c->nmz()->getnamestring(d->id).c_str(), wxConvUTF8) != devicenameCtrl->GetValue())
	{
		// Update device name
		if (devicenameCtrl->CheckValid(c, d->id, true))
		{
			d->id = c->nmz()->lookup(string(devicenameCtrl->GetValue().mb_str()));
			changedSomething = true;
		}
	}

	if (d->kind == aclock)
	{
		// Update half-time-period
		if (selectedDev->Get()->frequency != spinCtrl->GetValue())
		{
			selectedDev->Get()->frequency = spinCtrl->GetValue();
			changedSomething = true;
		}
	}
	else if (d->kind == andgate || d->kind == nandgate || d->kind == orgate || d->kind == norgate)
	{
		if (GetLinkedListLength(selectedDev->Get()->ilist) != spinCtrl->GetValue())
		{
			// Change number of gate inputs
			c->dmz()->SetGateInputCount(selectedDev->Get(), spinCtrl->GetValue());
			changedSomething = true;
		}
		if (selectedDev->Get()->kind != gateTypeDropdown->GetDevicekind())
		{
			// Change device type between and/nand/or/nor
			selectedDev->Get()->kind = gateTypeDropdown->GetDevicekind();
			devicekindStaticText->SetLabel(gateTypeDropdown->GetValue());
			changedSomething = true;
		}
	}

	if (changedSomething)
	{
		// Notify other bits of the GUI of any changes
		c->circuitChanged.Trigger();
		c->monitorsChanged.Trigger();
		UpdateApplyButtonState();
	}
}

void DeviceDetailsPanel::UpdateApplyButtonState()
{
	// Update "apply changes" button state - only enable if field values are different to current device properties
	updateBtn->Disable();
	if (c && selectedDev && selectedDev->Get())
	{
		devlink d = selectedDev->Get();

		if (wxString(c->nmz()->getnamestring(d->id).c_str(), wxConvUTF8) != devicenameCtrl->GetValue())
			updateBtn->Enable();

		if (d->kind == aclock)
		{
			if (d->frequency != spinCtrl->GetValue())
				updateBtn->Enable();
		}
		else if (d->kind == andgate || d->kind == nandgate || d->kind == orgate || d->kind == norgate)
		{
			if (GetLinkedListLength(d->ilist) != spinCtrl->GetValue())
				updateBtn->Enable();
			if (d->kind != gateTypeDropdown->GetDevicekind())
				updateBtn->Enable();
		}
	}
}

void DeviceDetailsPanel::OnInputChanged(wxCommandEvent& event)
{
	UpdateApplyButtonState();
}

BEGIN_EVENT_TABLE(DeviceDetailsPanel, wxPanel)
	EVT_TEXT(wxID_ANY, DeviceDetailsPanel::OnInputChanged)
	EVT_BUTTON(DEVICES_APPLY_BUTTON_ID, DeviceDetailsPanel::OnApply)
END_EVENT_TABLE()
