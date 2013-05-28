#include "gui-devices.h"
#include "network.h"
#include <algorithm>
using namespace std;

//typedef enum {aswitch, aclock, andgate, nandgate, orgate,norgate, xorgate, dtype, baddevice} devicekind;
string devicenamestrings[baddevice] = {"switch", "clock", "AND gate", "NAND gate", "OR gate", "NOR gate", "XOR gate", "DTYPE"};

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
	mainSizer = new wxBoxSizer(wxVERTICAL);
	devListBox = new DevicesListBox(c, selectedDev, this, wxID_ANY);
	deviceListSizer->Add(devListBox, 1, wxALL | wxEXPAND, 10);
	
	topsizer->Add(deviceListSizer, 1, wxEXPAND, 0);
	topsizer->Add(mainSizer, 4, wxEXPAND, 0);
	
	//topsizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
	SetSizerAndFit(topsizer);
	SetSizeHints(400,400);

	OnDeviceSelectionChanged();
}

DevicesDialog::~DevicesDialog()
{
	selectedDev->changed.Detach(this);
	devListBox->ReleasePointers();// detach from selectedDev->changed observer
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
	Layout();


	cout << "Changed";
	d = selectedDev->Get();
	if (d!=NULL)
	{
		cout << " to " << c->nmz()->getnamestring(d->id) << endl;
	}
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
	if (inputsPanel)
	{
		inputsPanel->Destroy();
		inputsPanel = NULL;
	}
}

BEGIN_EVENT_TABLE(DevicesDialog, wxDialog)

END_EVENT_TABLE()




DeviceDetailsPanel::DeviceDetailsPanel(circuit* circ, SelectedDevice* selectedDev_in, wxWindow* parent, wxWindowID id) :
	wxPanel(parent, id)
{
	selectedDev = selectedDev_in;
	c = circ;

	mainSizer = new wxStaticBoxSizer(wxVERTICAL, this, _("Device details"));
	wxBoxSizer* sizer1 = new wxBoxSizer(wxHORIZONTAL);
	//topsizer->Add(new wxButton(this, wxID_ANY, _("Test button")), 0, wxALL | wxEXPAND, 10);
	sizer1->Add(new wxStaticText(this, wxID_ANY, _("device info goes here")), 0, wxALL | wxEXPAND, 10);
	mainSizer->Add(sizer1);
	SetSizerAndFit(mainSizer);
}

BEGIN_EVENT_TABLE(DeviceDetailsPanel, wxPanel)

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
		devNames.Add(wxString(devs[i].devname.c_str(), wxConvUTF8));
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

BEGIN_EVENT_TABLE(DevicesListBox, wxListBox)
	EVT_LISTBOX(wxID_ANY, DevicesListBox::OnLBoxSelectionChanged)
END_EVENT_TABLE()
