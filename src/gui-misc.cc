#include "gui-misc.h"
#include <wx/msgdlg.h>
#include <algorithm>
using namespace std;

SwitchesCheckListBox::SwitchesCheckListBox(circuit* circ, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
	: wxCheckListBox(parent, id, pos, size, 0, NULL, style & ~wxLB_SORT)
{
	c = circ;
	if (c) c->circuitChanged.Attach(this, &SwitchesCheckListBox::OnCircuitChanged);
	OnCircuitChanged();
}

SwitchesCheckListBox::~SwitchesCheckListBox()
{
	if (c) c->circuitChanged.Detach(this);
}

void SwitchesCheckListBox::OnCircuitChanged()
{
	if (!c) return;

	// Update switch names
	switches.resize(0);
	devlink d = c->netz()->devicelist();
	while (d!=NULL)
	{
		if (d->kind == aswitch) switches.push_back(CircuitElementInfo(d));
		d = d->next;
	}
	switches.UpdateSignalNames(c);
	wxArrayString switchNames;
	CircuitElementInfoVector_to_wxArrayString(switches, switchNames);
	Set(switchNames);
	// Update switch states
	for (int i=0; i<switches.size(); i++)
	{
		Check(i, switches[i].d->swstate==high);
	}
}

void SwitchesCheckListBox::OnSwitchChanged(wxCommandEvent& event)
{
	if (!c) return;

	// Checkbox changed, update underlying switch state
	int i = event.GetInt();
	if (i>=switches.size()) 
	{
		ShowErrorMsgDialog(this,  _("Tried to change unknown switch"));
		return;
	}
	if (IsChecked(i))
		switches[i].d->swstate = high;
	else
		switches[i].d->swstate = low;

	c->circuitChanged.Trigger();
}

BEGIN_EVENT_TABLE(SwitchesCheckListBox, wxCheckListBox)
	EVT_CHECKLISTBOX(wxID_ANY, SwitchesCheckListBox::OnSwitchChanged)
END_EVENT_TABLE()


wxString devicekindstrings[baddevice] = {_("Switch"), _("Clock"), _("AND gate"), _("NAND gate"), _("OR gate"), _("NOR gate"), _("XOR gate"), _("D-type flip-flop"), _("Signal generator")};

DevicekindDropdown::DevicekindDropdown(wxWindow* parent, wxWindowID id, vector<devicekind> filterDevicekinds) :
	wxComboBox(parent, id, wxT(""), wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY)
{
	filter = filterDevicekinds;
	// Fill dropdown with available device kinds (optionally filtering to only include those listed in filterDevicekinds)
	for (int i=0; i<baddevice; i++)
	{
		if (filter.size()==0 || find(filter.begin(), filter.end(), devicekind(i)) != filter.end())
			Append(devicekindstrings[i]);
	}
}

devicekind DevicekindDropdown::GetDevicekind()
{
	for (int i=0; i<baddevice; i++)
	{
		if (devicekindstrings[i] == GetValue())
			return devicekind(i);
	}
	return baddevice;
}

void DevicekindDropdown::SetDevicekind(devicekind dk)
{
	if (dk>=0 && dk<baddevice)
	{
		if (filter.size()==0 || find(filter.begin(), filter.end(), dk) != filter.end())
			SetValue(devicekindstrings[dk]);
	}
}


DeviceNameTextCtrl::DeviceNameTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value) :
	wxTextCtrl(parent, id, value)
{}

bool DeviceNameTextCtrl::CheckValid(circuit* c, name allowExisting, bool errorDialog)
{
	if (!GetParent()) errorDialog = false;

	if (GetValue() == wxT(""))
	{
		if (errorDialog) ShowErrorMsgDialog(GetParent(), _("Device name cannot be blank"));
		return false;
	}
	if (!isalpha(GetValue()[0]))
	{
		if (errorDialog) ShowErrorMsgDialog(GetParent(), _("Device name must start with a letter"));
		return false;
	}
	if (!circuit::IsDeviceNameValid(string(GetValue().mb_str())))
	{
		if (errorDialog) ShowErrorMsgDialog(GetParent(), _("Device name must contain only letters and numbers"));
		return false;
	}
	if (c)
	{
		name newname = c->nmz()->cvtname(string(GetValue().mb_str()));
		if (newname!=blankname && newname<=lastreservedname)
		{
			if (errorDialog) ShowErrorMsgDialog(GetParent(), _("Device name cannot be a reserved word"));
			return false;
		}
		if (newname!=allowExisting && c->netz()->finddevice(newname)!=NULL)
		{
			if (errorDialog) ShowErrorMsgDialog(GetParent(), _("A device already exists with that name"));
			return false;
		}
	}
	return true;
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
	devs.push_back_all_devs(c->netz()->devicelist());
	devs.UpdateSignalNames(c);
	sort(devs.begin(), devs.end(), CircuitElementInfo_namestrcmp);

	int selectedIndex = -1;
	wxArrayString devNames;
	devNames.Alloc(devs.size());
	for (int i=0; i<devs.size(); i++)
	{
		if (devs[i].d==selectedDev->Get())
			selectedIndex = i;
		wxString desc(devs[i].namestr.c_str(), wxConvUTF8);
		if (!c->dmz()->CheckDeviceInputs(devs[i].d))
			desc = desc + wxT(" ") + _("(disconnected input)");
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


void ShowErrorMsgDialog(wxWindow* parent, wxString txt)
{
	wxMessageDialog dlg(parent, txt, _("Error"), wxCANCEL | wxICON_ERROR);
	dlg.ShowModal();
}

void CircuitElementInfoVector_to_wxArrayString(const CircuitElementInfoVector& src, wxArrayString& dest)
{
	dest.Clear();
	dest.Alloc(src.size());
	for (CircuitElementInfoVector::const_iterator it=src.begin(); it<src.end(); ++it)
	{
		dest.Add(wxString(it->namestr.c_str(), wxConvUTF8));
	}
}
