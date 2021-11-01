#include "Hooks.h"

#include "DiscoveryMusicManager.h"
#include "ImportManager.h"

void Hooks::Install()
{
	DiscoveryMusicManager::InstallHooks();
	ImportManager::InstallHooks();
}
