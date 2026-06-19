#include <obs-module.h>
#include <plugin-support.h>

#include "ui/ScoreboardDock.h"

#include <obs-frontend-api.h>

#include <string>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

namespace {

constexpr const char *DockId = "tennis-scoreboard-control-dock";
tennis_scoreboard::ScoreboardDock *scoreboardDock = nullptr;

} // namespace

bool obs_module_load(void)
{
	const char *dataPath = obs_get_module_data_path(obs_current_module());
	scoreboardDock = new tennis_scoreboard::ScoreboardDock(dataPath ? dataPath : "");
	if (!obs_frontend_add_dock_by_id(DockId, "Tennis Scoreboard", scoreboardDock)) {
		obs_log(LOG_WARNING, "dock registration failed; id may already be in use");
	}

	obs_log(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_frontend_remove_dock(DockId);
	scoreboardDock = nullptr;
	obs_log(LOG_INFO, "plugin unloaded");
}
