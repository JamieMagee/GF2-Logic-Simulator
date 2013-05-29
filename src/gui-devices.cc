#include "gui-devices.h"
#include "network.h"
#include "wx_icon.xpm"
#include <algorithm>
using namespace std;

//typedef enum {aswitch, aclock, andgate, nandgate, orgate,norgate, xorgate, dtype, baddevice} devicekind;
wxString devicenamestrings[baddevice] = {_("Switch"), _("Clock"), _("AND gate"), _("NAND gate"), _("OR gate"), _("NOR gate"), _("XOR gate"), _("D-type flip-flop")};

DevicesDialog::DevicesDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title, devlink d) :
	wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	c = circ;
	outputPanels.resize(0);
	inputsPanel = NULL;
	detailsPanel = NULL;
	selectedDev = new SelectedDevice();
	selectedDev->Set(d);
	selectedDev->changed.Attach(this, &DevicesDialog::OnDeviceSelectionChanged);

	wxBoxSizer* topsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* deviceListSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* ioSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer = new wxBoxSizer(wxVERTICAL);
	devListBox = new DevicesListBox(c, selectedDev, this, wxID_ANY);
	devListBox->SetMinSize(wxSize(150,-1));
	deviceListSizer->Add(devListBox, 1, wxALL | wxEXPAND, 10);

	inputsPanel = new DeviceInputsPanel(c, selectedDev, this);
	ioSizer->Add(inputsPanel, 1, wxEXPAND | wxALL, 5);
	outputsSizer = new wxBoxSizer(wxVERTICAL);
	ioSizer->Add(outputsSizer, 1, wxEXPAND | wxALL, 5);
	mainSizer->Add(ioSizer, 1, wxEXPAND | wxALL, 5);


	topsizer->Add(deviceListSizer, 0, wxEXPAND, 0);
	topsizer->Add(mainSizer, 3, wxEXPAND, 0);
	
	//topsizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
	SetSizerAndFit(topsizer);
	SetSizeHints(400,300);

	OnDeviceSelectionChanged();
}

DevicesDialog::~DevicesDialog()
{
	selectedDev->changed.Detach(this);
	devListBox->ReleasePointers();// detach from selectedDev->changed observer
	inputsPanel->ReleasePointers();
	delete selectedDev;
}

void DevicesDialog::OnDeviceSelectionChanged()
{
	DestroyDeviceWidgets();

	if (!selectedDev || !c) return;

	devicekind dk = baddevice;
	devlink d = selectedDev->Get();
	if (d) dk = d->kind;

	if (dk==xorgate || dk==dtype)
	{
		detailsPanel = new DeviceDetailsPanel(c, selectedDev, this);
	}
	else
	{
		detailsPanel = new DeviceDetailsPanel(c, selectedDev, this);
	}
	mainSizer->Insert(0, detailsPanel, 0, wxEXPAND | wxALL, 10);

	//outputsSizer

	Layout();
	Fit();
}

void DevicesDialog::DestroyDeviceWidgets()
{
	for (vector<DeviceOutputPanel*>::iterator it=outputPanels.begin(); it<outputPanels.end(); ++it)
	{
		(*it)->Destroy();
	}
	outputPanels.resize(0);
	if (detailsPanel)
	{
		detailsPanel->Destroy();
		detailsPanel = NULL;
	}
}

void DevicesDialog::OnDeleteButton(wxCommandEvent& event)
{
	if (!c || !selectedDev || !selectedDev->Get()) return;

	devlink d = selectedDev->Get();
	devlink newDev = devListBox->GetSelectionAfterDelete();
	selectedDev->Set(newDev);
	c->RemoveDevice(d);
}

BEGIN_EVENT_TABLE(DevicesDialog, wxDialog)
	EVT_BUTTON(DEVICES_DELETE_BUTTON_ID, DevicesDialog::OnDeleteButton)
END_EVENT_TABLE()


DeviceInputsPanel::DeviceInputsPanel(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id) :
	wxPanel(parent, id)
{
	selectedDev = selectedDev_in;
	c = circ;

	wxStaticBoxSizer* mainSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Inputs"));
	lbox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_NEEDED_SB | wxLB_EXTENDED);
	lbox->SetMinSize(wxSize(150,100));
	mainSizer->Add(lbox, 1, wxEXPAND | (wxALL & ~wxBOTTOM), 10);
	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
	btnAddConn = new wxButton(this, DEVICES_ADDCONN_BUTTON_ID, _("Connect"));
	btnDelConn = new wxButton(this, DEVICES_DELCONN_BUTTON_ID, _("Disconnect"));
	buttonsSizer->Add(btnAddConn, 1, wxEXPAND, 10);
	buttonsSizer->Add(btnDelConn, 1, wxEXPAND, 10);
	mainSizer->Add(buttonsSizer, 0, wxEXPAND | (wxALL & ~wxTOP) , 10);
	SetSizerAndFit(mainSizer);

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
	/*vector<inplink> oldSelected;
	for (int i=0; i<selections.GetCount(); i++)
	{
		if (selections[i]<inps.size()) oldSelected.push_back(inps[selections[i]]);
	}*/

	inps.resize(0);
	wxArrayString lboxData;
	if (selectedDev && c && selectedDev->Get())
	{
		inplink i = selectedDev->Get()->ilist;
		while (i != NULL)
		{
			inps.push_back(i);
			i = i->next;
		}
		reverse(inps.begin(), inps.end());
		lboxData.Alloc(inps.size());
		for (vector<inplink>::iterator it=inps.begin(); it<inps.end(); ++it)
		{
			inplink i = *it;
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

	/*for (int i=0; i<inps.size(); i++)
	{
		vector<inplink>::iterator it = find(oldSelected.begin(), oldSelected.end(), inps[i]);
		if (it<oldSelected.end()) lbox->SetSelection(i);
	}*/
	UpdateControlStates();
}

void DeviceInputsPanel::OnDeviceSelectionChanged()
{
	lbox->SetSelection(wxNOT_FOUND);
	if (c && selectedDev && selectedDev->Get())
	{
		UpdateInps();
		Show();
	}
	else
	{
		Hide();
	}
}

void DeviceInputsPanel::OnLBoxSelectionChanged(wxCommandEvent& event)
{
	UpdateControlStates();
}

void DeviceInputsPanel::UpdateControlStates()
{
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
		for (int i=0; i<selections.GetCount(); i++)
		{
			if (selections[i]<inps.size())
			{
				inps[selections[i]]->connect = dlg.result.o;
			}
		}
		c->circuitChanged.Trigger();
	}
}

void DeviceInputsPanel::OnDisconnectButton(wxCommandEvent& event)
{
	wxArrayInt selections;
	lbox->GetSelections(selections);
	for (int i=0; i<selections.GetCount(); i++)
	{
		if (i<inps.size()) inps[selections[i]]->connect = NULL;
	}
	UpdateInps();
}

BEGIN_EVENT_TABLE(DeviceInputsPanel, wxPanel)
	EVT_LISTBOX(wxID_ANY, DeviceInputsPanel::OnLBoxSelectionChanged)
	EVT_BUTTON(DEVICES_ADDCONN_BUTTON_ID, DeviceInputsPanel::OnConnectButton)
	EVT_BUTTON(DEVICES_DELCONN_BUTTON_ID, DeviceInputsPanel::OnDisconnectButton)
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
			kindtext = devicenamestrings[dk];
		
		gridsizer->Add(new wxStaticText(this, wxID_ANY, _("Device type:")), wxGBPosition(0,0), wxDefaultSpan, (wxALL & ~wxRIGHT) | wxALIGN_CENTER_VERTICAL, 10);
		gridsizer->Add(new wxStaticText(this, wxID_ANY, kindtext), wxGBPosition(0,1), wxDefaultSpan, wxALL | wxALIGN_CENTER_VERTICAL, 10);
		gridsizer->Add(new wxStaticText(this, wxID_ANY, _("Name:")), wxGBPosition(0,3), wxDefaultSpan, (wxALL & ~wxRIGHT) | wxALIGN_CENTER_VERTICAL, 10);
		devicenameCtrl = new wxTextCtrl(this, DEVICENAME_TEXTCTRL_ID, wxString(c->nmz()->getnamestring(d->id).c_str(), wxConvUTF8));
		gridsizer->Add(devicenameCtrl, wxGBPosition(0,4), wxDefaultSpan, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 10);
		gridsizer->AddGrowableCol(1,3);
		gridsizer->AddGrowableCol(4,5);
		mainSizer->Add(gridsizer, 0, wxEXPAND);

		CreateExtraFields();

		buttonsSizer->AddStretchSpacer();
		buttonsSizer->Add(new wxButton(this, DEVICES_DELETE_BUTTON_ID, _("Delete device")), 0, (wxALL & ~wxLEFT) | wxEXPAND, 10);
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
		name newname = c->nmz()->lookup(string(devicenameCtrl->GetValue().mb_str()));
		if (devicenameCtrl->GetValue() == wxT(""))
		{
			ShowErrorMsg(_("Device name cannot be blank"));
		}
		else if (!isalpha(devicenameCtrl->GetValue()[0]))
		{
			ShowErrorMsg(_("Device name must start with a letter"));
		}
		else if (!circuit::IsDeviceNameValid(string(devicenameCtrl->GetValue().mb_str())))
		{
			ShowErrorMsg(_("Device name must contain only letters and numbers"));
		}
		else if (newname<=lastreservedname)
		{
			ShowErrorMsg(_("Device name cannot be a reserved word"));
		}
		else if (c->netz()->finddevice(newname)!=NULL)
		{
			ShowErrorMsg(_("Could not rename device, as a device already exists with that name"));
		}
		else
		{
			d->id = newname;
			changedSomething = true;
		}
	}

	OnApply_ExtraFields(changedSomething);
	if (changedSomething)
	{
		c->circuitChanged.Trigger();
		c->monitorsChanged.Trigger();
		UpdateApplyButtonState();
	}
}

void DeviceDetailsPanel::ShowErrorMsg(wxString txt)
{
	wxMessageDialog dlg(this, txt, _("Error"), wxCANCEL | wxICON_ERROR);
	dlg.ShowModal();
}

void DeviceDetailsPanel::UpdateApplyButtonState()
{
	updateBtn->Disable();
	if (c && selectedDev && selectedDev->Get())
	{
		if (wxString(c->nmz()->getnamestring(selectedDev->Get()->id).c_str(), wxConvUTF8) != devicenameCtrl->GetValue())
			updateBtn->Enable();
		UpdateApplyButtonState_ExtraFields();
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




ChooseOutputDialog::ChooseOutputDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title, const wxString& description):
	wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	SetIcon(wxIcon(wx_icon));

	c = circ;

	devlink d = c->netz()->devicelist();
	while (d != NULL)
	{
		outplink o = d->olist;
		while (o != NULL)
		{
			outputs.push_back(CircuitElementInfo(o, c->netz()->getsignalstring(d,o)));
			o = o->next;
		}
		d = d->next;
	}
	sort(outputs.begin(), outputs.end(), CircuitElementInfo_namestrcmp);

	wxArrayString displayedOutputs;
	displayedOutputs.Alloc(outputs.size());
	for (vector<CircuitElementInfo>::iterator it=outputs.begin(); it<outputs.end(); ++it)
	{
		displayedOutputs.Add(wxString(it->namestr.c_str(), wxConvUTF8));
	}

	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
	topsizer->Add(new wxStaticText(this, wxID_ANY, description), 0, wxLEFT | wxRIGHT | wxTOP, 10);
	lbox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, displayedOutputs, wxLB_SINGLE | wxLB_NEEDED_SB);
	topsizer->Add(lbox, 1, wxALL | wxEXPAND, 10);
	topsizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
	SetSizerAndFit(topsizer);
}

void ChooseOutputDialog::OnOK(wxCommandEvent& event)
{
	int i = lbox->GetSelection();
	if (i>=0 && i<outputs.size())
	{
		result = outputs[i];
	}
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(ChooseOutputDialog, wxDialog)
	EVT_BUTTON(wxID_OK, ChooseOutputDialog::OnOK)
END_EVENT_TABLE()




bool DeviceInfo_namestrcmp(const DeviceInfo a, const DeviceInfo b)
{
	return (a.devname<b.devname);
}

DevicesListBox::DevicesListBox(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id) :
	wxListBox(parent, id, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_NEEDED_SB | wxLB_SINGLE)
{
	selectedDev = selectedDev_in;
	c = circ;
	isSetting = false;
	c->circuitChanged.Attach(this, &DevicesListBox::OnCircuitChanged);
	selectedDev->changed.Attach(this, &DevicesListBox::OnDeviceSelectionChanged);
	OnCircuitChanged();
}

DevicesListBox::~DevicesListBox()
{
	ReleasePointers();
}

void DevicesListBox::ReleasePointers()
{
	if (selectedDev)
		selectedDev->changed.Detach(this);
	selectedDev = NULL;
	if (c)
		c->circuitChanged.Detach(this);
	c = NULL;
}

void DevicesListBox::OnCircuitChanged()
{
	if (!c || !selectedDev) return;

	devs.resize(0);
	devlink d = c->netz()->devicelist();
	while (d!=NULL)
	{
		DeviceInfo dinf;
		dinf.d = d;
		dinf.devname = c->nmz()->getnamestring(d->id);
		devs.push_back(dinf);
		d = d->next;
	}
	sort(devs.begin(), devs.end(), DeviceInfo_namestrcmp);

	int selectedIndex = -1;
	wxArrayString devNames;
	devNames.Alloc(devs.size());
	for (int i=0; i<devs.size(); i++)
	{
		if (devs[i].d==selectedDev->Get())
			selectedIndex = i;
		wxString desc(devs[i].devname.c_str(), wxConvUTF8);
		inplink il = devs[i].d->ilist;
		while (il != NULL)
		{
			if (!il->connect)
			{
				desc = desc + wxT(" ") + _("(disconnected input)");
				break;
			}
			il = il->next;
		}
		devNames.Add(desc);
	}
	Set(devNames);

	if (selectedIndex<0)
	{
		if (devs.size()!=0)
			selectedDev->Set(devs[0].d);
		else
			selectedDev->Set(NULL);
	}
	else
	{
		SetSelection(selectedIndex);
	}
}

void DevicesListBox::OnDeviceSelectionChanged()
{
	if (!selectedDev || isSetting) return;

	for (int i=0; i<devs.size(); i++)
	{
		if (devs[i].d = selectedDev->Get())
		{
			SetSelection(i);
			return;
		}
	}
	SetSelection(wxNOT_FOUND);
}

void DevicesListBox::OnLBoxSelectionChanged(wxCommandEvent& event)
{
	if (!selectedDev) return;

	devlink d;
	int i = GetSelection();
	if (i>=0 && i<devs.size())
	{
		isSetting = true;// temporarily prevent DevicesListBox::OnDeviceSelectionChanged() from being run, otherwise the selection doesn't change properly
		selectedDev->Set(devs[i].d);
		isSetting = false;
	}
}

devlink DevicesListBox::GetSelectionAfterDelete()
{
	if (!c || !selectedDev || !selectedDev->Get())
	{
		if (devs.size()) return devs[0].d;
		else return NULL;
	}
	if (devs.size()<=1) return NULL;

	int i = GetSelection()+1;
	if (i>=devs.size()) i = devs.size()-1;
	if (i==GetSelection()) i = 0;
	return devs[i].d;
}

BEGIN_EVENT_TABLE(DevicesListBox, wxListBox)
	EVT_LISTBOX(wxID_ANY, DevicesListBox::OnLBoxSelectionChanged)
END_EVENT_TABLE()
