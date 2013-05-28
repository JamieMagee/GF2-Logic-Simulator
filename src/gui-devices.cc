#include "gui-devices.h"

//typedef enum {aswitch, aclock, andgate, nandgate, orgate,norgate, xorgate, dtype, baddevice} devicekind;
string devicenamestrings[baddevice] = {"switch", "clock", "AND gate", "NAND gate", "OR gate", "NOR gate", "XOR gate", "DTYPE"};

DevicesDialog::DevicesDialog(circuit* circ, wxWindow* parent, wxWindowID id, const wxString& title, devlink d) :
	wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	c = circ;
	if (d==NULL)
		d = c->netz()->devicelist();
	outputPanels.resize(0);
	inputsPanel = NULL;
	detailsPanel = NULL;
	selectedDev.changed.Attach(this, &DevicesDialog::OnDeviceSelect);

	wxBoxSizer* topsizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* deviceListSizer = new wxBoxSizer(wxVERTICAL);
	devListBox = new DevicesListBox(c, &selectedDev);

	//topsizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);
	SetSizerAndFit(topsizer);

	selectedDev.Set(d);
	
}

DevicesDialog::~DevicesDialog()
{
	selectedDev.changed.Detach(this);
}

void DevicesDialog::OnDeviceSelect()
{
	;
}

void DevicesDialog::DestroyDeviceWidgets()
{
	vector<DeviceOutputPanel*> outputPanels;
}

BEGIN_EVENT_TABLE(DevicesDialog, wxDialog)

END_EVENT_TABLE()


wxListBox(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, int n = 0, const wxString choices[] = NULL, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = "listBox")
