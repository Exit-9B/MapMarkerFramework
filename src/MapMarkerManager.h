#pragma once

class MapMarkerManager
{
public:
	static auto GetSingleton() -> MapMarkerManager*;

	void InstallHooks();

	void LoadAllMapMarkers();
	void LoadMapMarkersFromFile(std::filesystem::path a_file);
	auto GetDiscoveryMusic(RE::MARKER_TYPE a_type) -> const char*;

	inline static RE::BSFixedString DiscoveryMusicGeneric{ "MUSDiscoveryGeneric"sv };
	inline static RE::BSFixedString DiscoveryMusicCity{ "MUSDiscoveryCity"sv };
	inline static RE::BSFixedString DiscoveryMusicTown{ "MUSDiscoveryTown"sv };
	inline static RE::BSFixedString DiscoveryMusicDungeon{ "MUSDiscoveryDungeon"sv };

private:
	MapMarkerManager() = default;

	static void UpdateMapMarker(RE::TESObjectREFR* a_markerRef, RE::MARKER_TYPE a_icon);

	static auto AssignMusic(
		RE::BSFixedString* a_musicType,
		RE::MARKER_TYPE a_markerType) -> RE::BSFixedString*;

	std::unordered_map<RE::MARKER_TYPE, RE::BSFixedString> _discoveryMusic{
		{ RE::MARKER_TYPE::kCity, DiscoveryMusicCity },
		{ RE::MARKER_TYPE::kTown, DiscoveryMusicTown },
		{ RE::MARKER_TYPE::kCave, DiscoveryMusicDungeon },
		{ RE::MARKER_TYPE::kNordicRuins, DiscoveryMusicDungeon },
		{ RE::MARKER_TYPE::kDwemerRuin, DiscoveryMusicDungeon },
		{ RE::MARKER_TYPE::kDragonLair, DiscoveryMusicDungeon },
		{ RE::MARKER_TYPE::kMine, DiscoveryMusicDungeon },
	};
};
