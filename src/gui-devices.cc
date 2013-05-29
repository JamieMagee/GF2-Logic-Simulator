#include "gui-devices.h"
#include "gui-id.h"
#include "network.h"
#include "wx_icon.xpm"
#include <algorithm>
#include <climits>
using namespace std;


DevicesDialog::DevicesDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title, devlink d) :
	wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	c = circ;
	outputsSizer = NULL;
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
	deviceListSizer->Add(new wxButton(this, DEVICECREATE_BUTTON_ID, _("Add device")), 0, (wxALL & ~wxTOP) | wxEXPAND, 10);

	inputsPanel = new DeviceInputsPanel(c, selectedDev, this);
	ioSizer->Add(inputsPanel, 1, wxEXPAND | wxALL, 5);
	outputsSizer = new wxBoxSizer(wxVERTICAL);
	ioSizer->Add(outputsSizer, 1, wxEXPAND | wxALL, 0);
	mainSizer->Add(ioSizer, 1, wxEXPAND | wxALL, 5);


	topsizer->Add(deviceListSizer, 0, wxEXPAND, 0);
	topsizer->Add(mainSizer, 3, wxEXPAND, 0);
	
	mainSizer->Add(CreateButtonSizer(wxOK), 0, wxALL | wxEXPAND, 10);
	SetSizerAndFit(topsizer);
	SetSizeHints(400,300);

	OnDeviceSelectionChanged();
}

DevicesDialog::~DevicesDialog()
{
	// detach all widgets from selectedDev->changed observer before deleting selectedDev
	selectedDev->changed.Detach(this);
	devListBox->ReleasePointers();
	inputsPanel->ReleasePointers();
	for (vector<DeviceOutputPanel*>::iterator it=outputPanels.begin(); it<outputPanels.end(); ++it)
	{
		(*it)->ReleasePointers();
	}
	delete selectedDev;
}

void DevicesDialog::OnDeviceSelectionChanged()
{
	DestroyDeviceWidgets();

	if (!selectedDev || !c || !outputsSizer) return;

	devlink d = selectedDev->Get();

	detailsPanel = new DeviceDetailsPanel(c, selectedDev, this);
	mainSizer->Insert(0, detailsPanel, 0, wxEXPAND | wxALL, 10);

	CircuitElementInfoVector outputs;
	outplink o = d->olist;
	while (o != NULL)
	{
		outputs.push_back(CircuitElementInfo(o, c->nmz()->getnamestring(o->id)));
		o = o->next;
	}
	sort(outputs.begin(), outputs.end(), CircuitElementInfo_namestrcmp);
	for (CircuitElementInfoVector::iterator it=outputs.begin(); it<outputs.end(); ++it)
	{
		DeviceOutputPanel* opanel = new DeviceOutputPanel(c, it->o, this);
		outputPanels.push_back(opanel);
		outputsSizer->Add(opanel, 1, wxEXPAND | wxALL, 5);
	}

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

void DevicesDialog::OnCreateButton(wxCommandEvent& event)
{
	if (!c || !selectedDev) return;
	NewDeviceDialog dlg(c, this, wxID_ANY, _("Create new device"));
	if (dlg.ShowModal()==wxID_OK && dlg.newdev)
	{
		selectedDev->Set(dlg.newdev);
		c->circuitChanged.Trigger();
	}
}

BEGIN_EVENT_TABLE(DevicesDialog, wxDialog)
	EVT_BUTTON(DEVICEDELETE_BUTTON_ID, DevicesDialog::OnDeleteButton)
	EVT_BUTTON(DEVICECREATE_BUTTON_ID, DevicesDialog::OnCreateButton)
END_EVENT_TABLE()


ChooseOutputDialog::ChooseOutputDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title, const wxString& description):
	wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	SetIcon(wxIcon(wx_icon));

	c = circ;

	outputs.push_back_all_outputs(c->netz()->devicelist());
	outputs.UpdateSignalNames(c);
	sort(outputs.begin(), outputs.end(), CircuitElementInfo_namestrcmp);

	wxArrayString displayedOutputs;
	CircuitElementInfoVector_to_wxArrayString(outputs, displayedOutputs);

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


ConnectToOutputDialog::ConnectToOutputDialog(circuit* circ, outplink outp, wxWindow* parent, wxWindowID id, const wxString& title):
	wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	SetIcon(wxIcon(wx_icon));

	c = circ;
	o = outp;

	inputs.push_back_all_inputs(c->netz()->devicelist());
	inputs.UpdateSignalNames(c);
	sort(inputs.begin(), inputs.end(), CircuitElementInfo_iconnect_namestrcmp);

	wxArrayString lboxContents;
	lboxContents.Alloc(inputs.size());
	for (CircuitElementInfoVector::iterator it=inputs.begin(); it<inputs.end(); ++it)
	{
		wxString desc(it->namestr.c_str(), wxConvUTF8);
		if (it->i->connect)
		{
			desc = desc + wxT(" (") + _("connected to ") + wxString(c->netz()->getsignalstring(it->i->connect->dev->id, it->i->connect->id).c_str(), wxConvUTF8) + wxT(")");
		}
		else
		{
			desc = desc + _(" (unconnected)");
		}
		lboxContents.Add(desc);
	}

	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
	topsizer->Add(new wxStaticText(this, wxID_ANY, wxString(_("Choose input(s) to connect to ")) + wxString(c->netz()->getsignalstring(o->dev, o).c_str(), wxConvUTF8)), 0, wxLEFT | wxRIGHT | wxTOP, 10);
	lbox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, lboxContents, wxLB_EXTENDED | wxLB_NEEDED_SB);
	topsizer->Add(lbox, 1, wxALL | wxEXPAND, 10);
	topsizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
	SetSizerAndFit(topsizer);
}

void ConnectToOutputDialog::OnOK(wxCommandEvent& event)
{
	wxArrayInt selections;
	lbox->GetSelections(selections);
	for (int i=0; i<selections.GetCount(); i++)
	{
		if (selections[i]<inputs.size())
			inputs[selections[i]].i->connect = o;
	}
	c->circuitChanged.Trigger();
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(ConnectToOutputDialog, wxDialog)
	EVT_BUTTON(wxID_OK, ConnectToOutputDialog::OnOK)
END_EVENT_TABLE()


NewDeviceDialog::NewDeviceDialog(circuit* circ,wxWindow* parent, wxWindowID id, const wxString& title):
	wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	SetIcon(wxIcon(wx_icon));

	c = circ;

	wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
	topsizer->Add(new wxStaticText(this, wxID_ANY, _("New device name:")), 0, wxLEFT | wxRIGHT | wxTOP, 10);
	devicenameCtrl = new DeviceNameTextCtrl(this);
	topsizer->Add(devicenameCtrl, 0, wxALL | wxEXPAND, 10);
	topsizer->Add(new wxStaticText(this, wxID_ANY, _("Device type:")), 0, wxLEFT | wxRIGHT | wxTOP, 10);
	dkindDropdown = new DevicekindDropdown(this);
	dkindDropdown->SetSelection(0);
	topsizer->Add(dkindDropdown, 0, wxALL | wxEXPAND, 10);
	topsizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
	SetSizerAndFit(topsizer);
}

void NewDeviceDialog::OnOK(wxCommandEvent& event)
{
	if (!devicenameCtrl->CheckValid(c, blankname, true))
		return;

	devicekind dk = dkindDropdown->GetDevicekind();
	name devname = c->nmz()->lookup(string(devicenameCtrl->GetValue().mb_str()));
	bool ok;
	if (dk==aswitch)
		c->dmz()->makedevice(dk, devname, 1, ok);
	else if (dk==aclock)
		c->dmz()->makedevice(dk, devname, 5, ok);
	else
		c->dmz()->makedevice(dk, devname, 2, ok);

	newdev = c->netz()->finddevice(devname);
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(NewDeviceDialog, wxDialog)
	EVT_BUTTON(wxID_OK, NewDeviceDialog::OnOK)
END_EVENT_TABLE()

