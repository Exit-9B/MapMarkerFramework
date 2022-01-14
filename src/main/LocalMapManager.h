#pragma once

#include "RE/SpecialMarkerData.h"

class LocalMapManager
{
public:
	static auto GetSingleton() -> LocalMapManager*;

	static void InstallHooks();
	void Load();

	void AddLocationMarker(RE::BGSLocation* a_location, RE::MARKER_TYPE a_marker);
	void AddLocTypeMarker(RE::BGSKeyword* a_locType, RE::MARKER_TYPE a_marker);
	void AddVendorMarker(RE::BGSListForm* a_vendorList, RE::MARKER_TYPE a_marker);

	~LocalMapManager() = default;
	LocalMapManager(const LocalMapManager& other) = delete;
	LocalMapManager(LocalMapManager&& other) = delete;
	LocalMapManager& operator=(const LocalMapManager& other) = delete;
	LocalMapManager& operator=(LocalMapManager&& other) = delete;

private:
	LocalMapManager() = default;

	static RE::MARKER_TYPE GetSpecialMarkerType(RE::SpecialMarkerData* a_data);
	RE::BGSListForm* GetVendorList(RE::BGSLocation* a_location);

	std::unordered_map<RE::BGSLocation*, RE::BGSListForm*> _vendorLists;

	std::unordered_map<RE::BGSLocation*, RE::MARKER_TYPE> _locationMarkers;
	std::unordered_map<RE::BGSListForm*, RE::MARKER_TYPE> _vendorMarkers;
	std::map<RE::BGSKeyword*, RE::MARKER_TYPE> _locTypeMarkers;

	inline static REL::Relocation<decltype(GetSpecialMarkerType)> _GetSpecialMarkerType;
};
