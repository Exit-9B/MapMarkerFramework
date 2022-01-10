#include "DiscoveryMusicManager.h"
#include "Patch.h"
#include <xbyak/xbyak.h>

auto DiscoveryMusicManager::GetSingleton() -> DiscoveryMusicManager*
{
	static DiscoveryMusicManager singleton{};
	return std::addressof(singleton);
}

void DiscoveryMusicManager::SetMusic(RE::MARKER_TYPE a_markerType, std::string_view a_musicType)
{
	if (GetMusic(a_markerType) == a_musicType) {
		return;
	}
	else if (a_musicType == DiscoveryMusic::Generic) {
		_lookup.erase(a_markerType);
		return;
	}

	if (!RE::TESForm::LookupByEditorID<RE::BGSMusicType>(a_musicType)) {
		logger::warn("'{}' did not correspond to a valid Music Type"sv, a_musicType);
		// note: allow it to load anyway to help people catch errors
	}

	_lookup[a_markerType] = a_musicType;
}

auto DiscoveryMusicManager::GetMusic(RE::MARKER_TYPE a_markerType) -> std::string_view
{
	auto it = _lookup.find(a_markerType);
	if (it != _lookup.end()) {
		return it->second;
	}
	else {
		return DiscoveryMusic::Generic;
	}
}

void DiscoveryMusicManager::InstallHooks()
{
	if (Patch::WriteDiscoveryMusicPatch(AssignMusic)) {
		logger::info("Installed hook for discovery music"sv);
	}
}

auto DiscoveryMusicManager::AssignMusic(
	RE::BSFixedString* a_musicType,
	RE::MARKER_TYPE a_markerType) -> RE::BSFixedString*
{
	*a_musicType = GetSingleton()->GetMusic(a_markerType);
	return a_musicType;
}
