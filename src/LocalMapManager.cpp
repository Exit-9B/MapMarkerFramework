#include "LocalMapManager.h"

auto LocalMapManager::GetSingleton() -> LocalMapManager*
{
	static LocalMapManager singleton{};
	return std::addressof(singleton);
}

void LocalMapManager::InstallHooks()
{
	auto& trampoline = SKSE::GetTrampoline();

	auto door_hook = REL::Relocation<std::uintptr_t>{ Offset::LocalMapMenu::PopulateData, 0x941 };
	_GetSpecialMarkerType = trampoline.write_call<5>(door_hook.address(), GetSpecialMarkerType);

	logger::info("Installed hooks for local map"sv);
}

void LocalMapManager::Load()
{
	const auto dataHandler = RE::TESDataHandler::GetSingleton();

	if (dataHandler) {
		auto& factions = dataHandler->GetFormArray<RE::TESFaction>();
		for (auto& faction : factions) {
			if (!faction || !faction->IsVendor())
				continue;

			auto vendorList = faction->vendorData.vendorSellBuyList;
			auto vendorChest = faction->vendorData.merchantContainer;
			auto location = vendorChest ? vendorChest->GetEditorLocation() : nullptr;

			if (vendorList && location) {
				_vendorLists[location] = vendorList;
			}
		}
	}
}

void LocalMapManager::AddLocationMarker(RE::BGSLocation* a_location, RE::MARKER_TYPE a_marker)
{
	_locationMarkers[a_location] = a_marker;
}

void LocalMapManager::AddLocTypeMarker(RE::BGSKeyword* a_locType, RE::MARKER_TYPE a_marker)
{
	_locTypeMarkers[a_locType] = a_marker;
}

void LocalMapManager::AddVendorMarker(RE::BGSListForm* a_vendorList, RE::MARKER_TYPE a_marker)
{
	_vendorMarkers[a_vendorList] = a_marker;
}

auto LocalMapManager::GetSpecialMarkerType(SpecialMarkerData* a_data) -> RE::MARKER_TYPE
{
	auto type = _GetSpecialMarkerType(a_data);

	constexpr RE::MARKER_TYPE kDoor = static_cast<RE::MARKER_TYPE>(61);

	if (type != kDoor) {
		return type;
	}

	RE::NiPointer<RE::TESObjectREFR> objectRef;
	RE::LookupReferenceByHandle(a_data->refHandle, objectRef);

	auto teleport = objectRef ? objectRef->extraList.GetByType<RE::ExtraTeleport>() : nullptr;

	if (!teleport) {
		return type;
	}

	auto linkedDoor = teleport->teleportData->linkedDoor.get();
	auto location = linkedDoor->GetEditorLocation();
	auto locTypeStore = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("LocTypeStore"sv);

	if (location) {
		auto importManager = GetSingleton();

		auto locationMarker = importManager->_locationMarkers.find(location);
		if (locationMarker != importManager->_locationMarkers.end()) {
			return locationMarker->second;
		}

		if (location->HasKeyword(locTypeStore)) {

			auto vendorList = importManager->GetVendorList(location);

			if (vendorList) {

				auto vendorMarker = importManager->_vendorMarkers.find(vendorList);
				if (vendorMarker != importManager->_vendorMarkers.end()) {
					return vendorMarker->second;
				}
			}
		}

		for (auto& [locKeyword, markerType] : importManager->_locTypeMarkers) {

			if (location->HasKeyword(locKeyword)) {
				return markerType;
			}
		}
	}

	return type;
}

auto LocalMapManager::GetVendorList(RE::BGSLocation* a_location) -> RE::BGSListForm*
{
	auto it = _vendorLists.find(a_location);
	return it != _vendorLists.end() ? it->second : nullptr;
}
