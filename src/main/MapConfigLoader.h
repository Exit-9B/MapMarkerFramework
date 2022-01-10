#pragma once

class MapConfigLoader
{
public:
	static auto GetSingleton() -> MapConfigLoader*;

	void LoadAll();
	void UpdateMarkers(std::uint32_t a_customIconIndex) const;

private:
	using MapMarker = std::variant<RE::MARKER_TYPE, std::string>;

	void LoadFromFile(
		const std::string& a_fileName,
		RE::BSResourceNiBinaryStream& a_fileStream);

	auto ResolveMarker(MapMarker a_marker, std::uint32_t a_customIconIndex) const -> RE::MARKER_TYPE;

	static void UpdateMapMarker(RE::TESObjectREFR* a_markerRef, RE::MARKER_TYPE a_icon);

	std::int32_t _lastIcon{ 0 };

	std::unordered_map<std::string, std::int32_t> _iconNames;
	std::unordered_map<std::string, std::string> _discoveryMusic;

	std::unordered_map<RE::TESObjectREFR*, MapMarker> _mapMarkers;
	std::unordered_map<RE::BGSLocation*, MapMarker> _locationMarkers;
	std::unordered_map<RE::BGSListForm*, MapMarker> _vendorMarkers;
	std::unordered_map<RE::BGSKeyword*, MapMarker> _locTypeMarkers;
};
