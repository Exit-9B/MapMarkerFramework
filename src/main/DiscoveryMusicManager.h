#pragma once

namespace DiscoveryMusic
{
	inline static RE::BSFixedString Generic{ "MUSDiscoveryGeneric"sv };
	inline static RE::BSFixedString City{ "MUSDiscoveryCity"sv };
	inline static RE::BSFixedString Town{ "MUSDiscoveryTown"sv };
	inline static RE::BSFixedString Dungeon{ "MUSDiscoveryDungeon"sv };
}

class DiscoveryMusicManager
{
public:
	static auto GetSingleton() -> DiscoveryMusicManager*;

	static void InstallHooks();

	void SetMusic(RE::MARKER_TYPE a_markerType, std::string_view a_musicType);
	auto GetMusic(RE::MARKER_TYPE a_markerType) -> std::string_view;

private:
	DiscoveryMusicManager() = default;

	static auto AssignMusic(
		RE::BSFixedString* a_musicType,
		RE::MARKER_TYPE a_markerType) -> RE::BSFixedString*;

	std::unordered_map<RE::MARKER_TYPE, RE::BSFixedString> _lookup{
		{ RE::MARKER_TYPE::kCity, DiscoveryMusic::City },
		{ RE::MARKER_TYPE::kTown, DiscoveryMusic::Town },
		{ RE::MARKER_TYPE::kCave, DiscoveryMusic::Dungeon },
		{ RE::MARKER_TYPE::kNordicRuin, DiscoveryMusic::Dungeon },
		{ RE::MARKER_TYPE::kDwemerRuin, DiscoveryMusic::Dungeon },
		{ RE::MARKER_TYPE::kDragonLair, DiscoveryMusic::Dungeon },
		{ RE::MARKER_TYPE::kMine, DiscoveryMusic::Dungeon },
	};
};
