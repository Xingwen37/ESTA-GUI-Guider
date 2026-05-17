#ifndef SIM_FEED_H
#define SIM_FEED_H

#include "app/app_main.h"
#include "sim_scenario.h"

void SimFeed_Init(App_MainState *app, SimScenarioRuntime *scenario);
bool SimFeed_Update(App_MainState *app, SimScenarioRuntime *scenario);
void SimFeed_Tick(SimScenarioRuntime *scenario);

#endif
