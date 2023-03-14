#pragma once

class MapConfigLoader
{
public:
	using MapMarker = std::variant<RE::MARKER_TYPE, std::string>;
	inline static const MapMarker NoMarker = RE::MARKER_TYPE::kNone;
	inline static const MapMarker DoorMarker = RE::MARKER_TYPE::kDoor;

	~MapConfigLoader() = default;
	MapConfigLoader(const MapConfigLoader&) = delete;
	MapConfigLoader(MapConfigLoader&&) = delete;
	MapConfigLoader& operator=(const MapConfigLoader&) = delete;
	MapConfigLoader& operator=(MapConfigLoader&&) = delete;

	static auto GetSingleton() -> MapConfigLoader*;

	void LoadAll();
	void UpdateMarkers(std::uint32_t a_customIconIndex) const;

	const MapMarker& GetMapMarker(RE::TESObjectREFR* a_refr) const;

	const MapMarker& GetLocalMarker(RE::BGSLocation* a_location) const;

	std::int32_t GetIconIndex(const std::string& a_name) const;

private:
	MapConfigLoader() = default;

	void LoadFromFile(
		const std::string& a_fileName,
		RE::BSResourceNiBinaryStream& a_fileStream);

	auto ResolveMarker(MapMarker a_marker, std::uint32_t a_customIconIndex) const -> RE::MARKER_TYPE;

	static void UpdateMapMarker(RE::TESObjectREFR* a_markerRef, RE::MARKER_TYPE a_icon);

	std::int32_t _lastIcon{ 0 };

	std::map<std::string, std::int32_t> _iconNames;
	std::map<std::string, std::string> _discoveryMusic;

	std::unordered_map<RE::TESObjectREFR*, MapMarker> _mapMarkers;
	std::unordered_map<RE::BGSLocation*, MapMarker> _locationMarkers;
	tsl::ordered_map<RE::BGSListForm*, MapMarker> _vendorMarkers;
	tsl::ordered_map<RE::BGSKeyword*, MapMarker> _locTypeMarkers;
};
