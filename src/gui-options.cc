#include "gui-options.h"
#include <wx/config.h>

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
}
