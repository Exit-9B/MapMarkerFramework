#pragma once

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

	// We don't know much about this struct; there is a BSTArray of it in LocalMapMenu
	struct SpecialMarkerData
	{
		std::uint64_t unk00;      // 00
		RE::RefHandle refHandle;  // 08
		std::uint32_t unk0C;      // 0C
		std::uint64_t unk10;      // 10
		std::int32_t unk18;       // 18 - 1 = quest, 2 = player set
		std::int32_t unk1C;       // 1C - 1 = door
		std::int32_t unk20;       // 20
		std::uint32_t unk24;      // 24
		std::uint64_t unk28;      // 28
		std::uint64_t unk30;      // 38
	};
	static_assert(sizeof(SpecialMarkerData) == 0x38);

	static RE::MARKER_TYPE GetSpecialMarkerType(SpecialMarkerData* a_data);
	RE::BGSListForm* GetVendorList(RE::BGSLocation* a_location);

	std::unordered_map<RE::BGSLocation*, RE::BGSListForm*> _vendorLists;

	std::unordered_map<RE::BGSLocation*, RE::MARKER_TYPE> _locationMarkers;
	std::unordered_map<RE::BGSListForm*, RE::MARKER_TYPE> _vendorMarkers;
	std::map<RE::BGSKeyword*, RE::MARKER_TYPE> _locTypeMarkers;

	inline static REL::Relocation<decltype(GetSpecialMarkerType)> _GetSpecialMarkerType;
};
