#include "gui-options.h"
#include "gui-id.h"
#include <wx/config.h>
#include <wx/gbsizer.h>
#include <wx/statbox.h>

LogsimOptions::LogsimOptions()
{
	Load();
	optionsChanged.Attach(this, &LogsimOptions::Save);
}

LogsimOptions::~LogsimOptions()
{
	optionsChanged.Detach(this);
}

void LogsimOptions::Save()
{
	wxConfig *config = new wxConfig(wxT("GF2Logsim-2013-8"));
	config->Write(wxT("xScaleMin"), xScaleMin);
	config->Write(wxT("xScaleMax"), xScaleMax);
	config->Write(wxT("sigHeightMin"), sigHeightMin);
	config->Write(wxT("sigHeightMax"), sigHeightMax);
	config->Write(wxT("continuousRate"), continuousRate);
	config->Write(wxT("debugMachineCycles"), debugMachineCycles);
	config->Write(wxT("debugSim"), debugSim);
	delete config;
}

void LogsimOptions::Load()
{
	ResetOptions();
	wxConfig *config = new wxConfig(wxT("GF2Logsim-2013-8"));
	config->Read(wxT("xScaleMin"), &xScaleMin);
	config->Read(wxT("xScaleMax"), &xScaleMax);
	config->Read(wxT("sigHeightMin"), &sigHeightMin);
	config->Read(wxT("sigHeightMax"), &sigHeightMax);
	config->Read(wxT("continuousRate"), &continuousRate);
	config->Read(wxT("debugMachineCycles"), &debugMachineCycles);
	config->Read(wxT("debugSim"), &debugSim);
	delete config;
}

void LogsimOptions::ResetOptions()
{
	xScaleMin = 2;
	xScaleMax = 40;
	sigHeightMin = 50;
	sigHeightMax = 200;
	continuousRate = 50;
	debugMachineCycles = false;
	debugSim = false;
}

OptionsDialog::OptionsDialog(LogsimOptions* options_in, wxWindow* parent, wxWindowID id, const wxString& title) :
	wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	options = options_in;
	wxStaticBoxSizer* debugBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Debugging"));
	wxStaticBoxSizer* tracesBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Trace size"));
	wxStaticBoxSizer* miscBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Misc"));
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* optionsSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
	wxGridBagSizer* tracesSizer = new wxGridBagSizer();
	wxGridBagSizer* miscSizer = new wxGridBagSizer();
	int row = 0;
	xScaleMinCtrl = new wxSpinCtrl(this, OPTIONS_XSCALE_MIN_ID);
	xScaleMinCtrl->SetRange(2,1000);
	xScaleMinCtrl->SetValue(options->xScaleMin);
	xScaleMaxCtrl = new wxSpinCtrl(this, OPTIONS_XSCALE_MAX_ID);
	xScaleMaxCtrl->SetRange(2,1000);
	xScaleMaxCtrl->SetValue(options->xScaleMax);
	sigHeightMinCtrl = new wxSpinCtrl(this, OPTIONS_SIGHEIGHT_MIN_ID);
	sigHeightMinCtrl->SetRange(30,1000);
	sigHeightMinCtrl->SetValue(options->sigHeightMin);
	sigHeightMaxCtrl = new wxSpinCtrl(this, OPTIONS_SIGHEIGHT_MAX_ID);
	sigHeightMaxCtrl->SetRange(30,1000);
	sigHeightMaxCtrl->SetValue(options->sigHeightMax);

	tracesSizer->Add(new wxStaticText(this, wxID_ANY, _("Min horizontal scale (pixels per cycle):")), wxGBPosition(row,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxALIGN_RIGHT, 1);
	tracesSizer->Add(xScaleMinCtrl, wxGBPosition(row,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 1);
	row++;
	tracesSizer->Add(new wxStaticText(this, wxID_ANY, _("Max horizontal scale (pixels per cycle):")), wxGBPosition(row,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxALIGN_RIGHT, 1);
	tracesSizer->Add(xScaleMaxCtrl, wxGBPosition(row,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 1);
	row++;
	tracesSizer->Add(new wxStaticText(this, wxID_ANY, _("Min signal height (pixels):")), wxGBPosition(row,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxALIGN_RIGHT, 1);
	tracesSizer->Add(sigHeightMinCtrl, wxGBPosition(row,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 1);
	row++;
	tracesSizer->Add(new wxStaticText(this, wxID_ANY, _("Max signal height (pixels):")), wxGBPosition(row,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxALIGN_RIGHT, 1);
	tracesSizer->Add(sigHeightMaxCtrl, wxGBPosition(row,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 1);
	row++;
	tracesSizer->AddGrowableCol(0,5);
	tracesSizer->AddGrowableCol(1,2);
	tracesBox->Add(tracesSizer, 0, wxEXPAND | wxALL, 10);
	leftSizer->Add(tracesBox, 0, wxEXPAND | wxALL, 5);

	row = 0;
	continuousRateCtrl = new wxSpinCtrl(this, OPTIONS_CONTRATE_ID);
	continuousRateCtrl->SetRange(1,100000);
	continuousRateCtrl->SetValue(options->continuousRate);
	miscSizer->Add(new wxStaticText(this, wxID_ANY, _("Continuous run rate (Hz):")), wxGBPosition(row,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL, 1);
	miscSizer->Add(continuousRateCtrl, wxGBPosition(row,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 1);
	miscSizer->AddGrowableCol(0,5);
	miscSizer->AddGrowableCol(1,2);
	miscBox->Add(miscSizer, 0, wxEXPAND | wxALL, 5);
	miscSizer->Layout();
	rightSizer->Add(miscBox, 0, wxEXPAND | wxALL, 5);
	
	row = 0;
	debugMachineCyclesCtrl = new wxCheckBox(this, OPTIONS_DEBUG_MACHINECYCLES_ID, _("Show machine cycles in monitor traces"));
	debugMachineCyclesCtrl->SetValue(options->debugMachineCycles);
	debugBox->Add(debugMachineCyclesCtrl, 0, wxALIGN_CENTER_VERTICAL, 10);
	debugSimCtrl = new wxCheckBox(this, OPTIONS_DEBUG_SIMULATION_ID, _("Print simulation debug information"));
	debugSimCtrl->SetValue(options->debugSim);
	debugBox->Add(debugSimCtrl, 0, wxALIGN_CENTER_VERTICAL, 10);
	rightSizer->Add(debugBox, 0, wxEXPAND | wxALL, 10);

	optionsSizer->Add(leftSizer, 1, wxEXPAND | wxALL, 0);
	optionsSizer->Add(rightSizer, 1, wxEXPAND | wxALL, 0);
	mainSizer->Add(optionsSizer, 0, wxEXPAND | wxALL, 0);

	mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL | wxEXPAND, 10);

	SetSizerAndFit(mainSizer);
}

void OptionsDialog::OnInputChanged(wxCommandEvent& event)
{
	if (event.GetId()==OPTIONS_XSCALE_MIN_ID && xScaleMinCtrl->GetValue() > xScaleMaxCtrl->GetValue())
		xScaleMaxCtrl->SetValue(xScaleMinCtrl->GetValue());
	if (event.GetId()==OPTIONS_XSCALE_MAX_ID && xScaleMinCtrl->GetValue() > xScaleMaxCtrl->GetValue())
		xScaleMinCtrl->SetValue(xScaleMaxCtrl->GetValue());
	if (event.GetId()==OPTIONS_SIGHEIGHT_MIN_ID && sigHeightMinCtrl->GetValue() > sigHeightMaxCtrl->GetValue())
		sigHeightMaxCtrl->SetValue(sigHeightMinCtrl->GetValue());
	if (event.GetId()==OPTIONS_SIGHEIGHT_MAX_ID && sigHeightMinCtrl->GetValue() > sigHeightMaxCtrl->GetValue())
		sigHeightMinCtrl->SetValue(sigHeightMaxCtrl->GetValue());
}

void OptionsDialog::OnOK(wxCommandEvent& event)
{
	bool changedSomething = false;
	if (xScaleMinCtrl->GetValue() != options->xScaleMin)
	{
		options->xScaleMin = xScaleMinCtrl->GetValue();
		changedSomething = true;
	}
	if (xScaleMaxCtrl->GetValue() != options->xScaleMax)
	{
		options->xScaleMax = xScaleMaxCtrl->GetValue();
		changedSomething = true;
	}
	if (sigHeightMinCtrl->GetValue() != options->sigHeightMin)
	{
		options->sigHeightMin = sigHeightMinCtrl->GetValue();
		changedSomething = true;
	}
	if (sigHeightMaxCtrl->GetValue() != options->sigHeightMax)
	{
		options->sigHeightMax = sigHeightMaxCtrl->GetValue();
		changedSomething = true;
	}

	if (options->xScaleMin > options->xScaleMax)
	{
		options->xScaleMin = options->xScaleMax;
		changedSomething = true;
	}
	if (options->sigHeightMin > options->sigHeightMax)
	{
		options->sigHeightMin = options->sigHeightMax;
		changedSomething = true;
	}

	if (continuousRateCtrl->GetValue() != options->continuousRate)
	{
		options->continuousRate = continuousRateCtrl->GetValue();
		changedSomething = true;
	}

	if (debugMachineCyclesCtrl->IsChecked() != options->debugMachineCycles)
	{
		options->debugMachineCycles = debugMachineCyclesCtrl->IsChecked();
		if (options->debugMachineCycles)
		{
			cout << "Machine cycles debug enabled." << endl;
			cout << "A red dashed line with a red simulation cycle number below and to the left indicates the end of that simulation cycle." << endl;
		}
		changedSomething = true;
	}
	if (debugSimCtrl->IsChecked() != options->debugSim)
	{
		options->debugSim = debugSimCtrl->IsChecked();
		changedSomething = true;
	}
	if (changedSomething)
	{
		options->optionsChanged.Trigger();
	}
	if (event.GetId()==wxID_OK) EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(OptionsDialog, wxDialog)
	EVT_BUTTON(wxID_OK, OptionsDialog::OnOK)
	EVT_TEXT(wxID_ANY, OptionsDialog::OnInputChanged)
END_EVENT_TABLE()

