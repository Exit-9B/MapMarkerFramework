#pragma once

class MapConfigLoader
{
public:
	void LoadAll();

private:
	using MapMarker = std::variant<RE::MARKER_TYPE, std::string>;

	void LoadFromFile(std::filesystem::path a_file);
	void UpdateMarkers();
	auto ResolveMarker(MapMarker a_marker) -> RE::MARKER_TYPE;

	static void UpdateMapMarker(RE::TESObjectREFR* a_markerRef, RE::MARKER_TYPE a_icon);

	std::unordered_map<std::string, RE::MARKER_TYPE> _iconNames;

	std::unordered_map<RE::TESObjectREFR*, MapMarker> _mapMarkers;
};
