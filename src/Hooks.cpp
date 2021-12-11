#include "Hooks.h"

#include "DiscoveryMusicManager.h"
#include "ImportManager.h"
#include "LocalMapManager.h"

void Hooks::Install()
{
	DiscoveryMusicManager::InstallHooks();
	ImportManager::InstallHooks();
	LocalMapManager::InstallHooks();
}
