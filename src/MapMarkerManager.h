#pragma once

class MapMarkerManager
{
public:
	static auto GetSingleton() -> MapMarkerManager*;

	void InstallHooks();

	void LoadAllMapMarkers();
	void LoadMapMarkersFromFile(std::filesystem::path a_file);
	auto GetDiscoveryMusic(RE::MARKER_TYPE a_type) -> const char*;

	inline static std::string DiscoveryMusicGeneric{ "MUSDiscoveryGeneric"s };
	inline static std::string DiscoveryMusicCity{ "MUSDiscoveryCity"s };
	inline static std::string DiscoveryMusicTown{ "MUSDiscoveryTown"s };
	inline static std::string DiscoveryMusicDungeon{ "MUSDiscoveryDungeon"s };

private:
	MapMarkerManager() = default;

	static void UpdateMapMarker(RE::TESObjectREFR* a_markerRef, RE::MARKER_TYPE a_icon);

	static auto AssignMusic(
		RE::BSFixedString* a_musicType,
		RE::MARKER_TYPE a_markerType) -> RE::BSFixedString*;

	std::unordered_map<RE::MARKER_TYPE, std::string> _discoveryMusic{
		{ RE::MARKER_TYPE::kCity, DiscoveryMusicCity },
		{ RE::MARKER_TYPE::kTown, DiscoveryMusicTown },
		{ RE::MARKER_TYPE::kCave, DiscoveryMusicDungeon },
		{ RE::MARKER_TYPE::kNordicRuins, DiscoveryMusicDungeon },
		{ RE::MARKER_TYPE::kDwemerRuin, DiscoveryMusicDungeon },
		{ RE::MARKER_TYPE::kDragonLair, DiscoveryMusicDungeon },
		{ RE::MARKER_TYPE::kMine, DiscoveryMusicDungeon },
	};
};
