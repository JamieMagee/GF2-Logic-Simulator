#include "gui-options.h"
#include "gui-id.h"
#include <wx/config.h>
#include <wx/gbsizer.h>
#include <wx/statbox.h>

LogsimOptions::LogsimOptions()
{
	ResetOptions();
}

void LogsimOptions::ResetOptions()
{
	xScaleMin = 2;
	xScaleMax = 40;
	spacingMin = 50;
	spacingMax = 200;
	continuousRate = 50;
	debugMachineCycles = false;
	debugSim = false;
	optionsChanged.Trigger();
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
	/* 	OPTIONS_XSCALE_MIN_ID,
	OPTIONS_XSCALE_MAX_ID,
	OPTIONS_SPACING_MIN_ID,
	OPTIONS_SPACING_MAX_ID,
	OPTIONS_DEBUG_MACHINECYCLES_ID,
	OPTIONS_DEBUG_SIMULATION_ID */
	int row = 0;
	xScaleMinCtrl = new wxSpinCtrl(this, OPTIONS_XSCALE_MIN_ID);
	xScaleMinCtrl->SetRange(2,1000);
	xScaleMinCtrl->SetValue(options->xScaleMin);
	xScaleMaxCtrl = new wxSpinCtrl(this, OPTIONS_XSCALE_MAX_ID);
	xScaleMaxCtrl->SetRange(2,1000);
	xScaleMaxCtrl->SetValue(options->xScaleMax);
	spacingMinCtrl = new wxSpinCtrl(this, OPTIONS_SPACING_MIN_ID);
	spacingMinCtrl->SetRange(30,1000);
	spacingMinCtrl->SetValue(options->spacingMin);
	spacingMaxCtrl = new wxSpinCtrl(this, OPTIONS_SPACING_MAX_ID);
	spacingMaxCtrl->SetRange(30,1000);
	spacingMaxCtrl->SetValue(options->spacingMax);

	tracesSizer->Add(new wxStaticText(this, wxID_ANY, _("Min horizontal scale (pixels per cycle):")), wxGBPosition(row,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxALIGN_RIGHT, 1);
	tracesSizer->Add(xScaleMinCtrl, wxGBPosition(row,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 1);
	row++;
	tracesSizer->Add(new wxStaticText(this, wxID_ANY, _("Max horizontal scale (pixels per cycle):")), wxGBPosition(row,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxALIGN_RIGHT, 1);
	tracesSizer->Add(xScaleMaxCtrl, wxGBPosition(row,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 1);
	row++;
	tracesSizer->Add(new wxStaticText(this, wxID_ANY, _("Min vertical spacing (pixels):")), wxGBPosition(row,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxALIGN_RIGHT, 1);
	tracesSizer->Add(spacingMinCtrl, wxGBPosition(row,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 1);
	row++;
	tracesSizer->Add(new wxStaticText(this, wxID_ANY, _("Max vertical spacing (pixels):")), wxGBPosition(row,0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxALIGN_RIGHT, 1);
	tracesSizer->Add(spacingMaxCtrl, wxGBPosition(row,1), wxDefaultSpan, wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 1);
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
	if (event.GetId()==OPTIONS_SPACING_MIN_ID && spacingMinCtrl->GetValue() > spacingMaxCtrl->GetValue())
		spacingMaxCtrl->SetValue(spacingMinCtrl->GetValue());
	if (event.GetId()==OPTIONS_SPACING_MAX_ID && spacingMinCtrl->GetValue() > spacingMaxCtrl->GetValue())
		spacingMinCtrl->SetValue(spacingMaxCtrl->GetValue());
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
	if (spacingMinCtrl->GetValue() != options->spacingMin)
	{
		options->spacingMin = spacingMinCtrl->GetValue();
		changedSomething = true;
	}
	if (spacingMaxCtrl->GetValue() != options->spacingMax)
	{
		options->spacingMax = spacingMaxCtrl->GetValue();
		changedSomething = true;
	}

	if (options->xScaleMin > options->xScaleMax)
	{
		options->xScaleMin = options->xScaleMax;
		changedSomething = true;
	}
	if (options->spacingMin > options->spacingMax)
	{
		options->xScaleMin = options->xScaleMax;
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
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(OptionsDialog, wxDialog)
	EVT_BUTTON(wxID_OK, OptionsDialog::OnOK)
	EVT_TEXT(wxID_ANY, OptionsDialog::OnInputChanged)
END_EVENT_TABLE()

