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
				vendorLists[location] = vendorList;
			}
		}
	}
}

void LocalMapManager::AddLocTypeMarker(RE::BGSKeyword* a_locType, RE::MARKER_TYPE a_marker)
{
	locTypeMarkers[a_locType] = a_marker;
}

void LocalMapManager::AddVendorMarker(RE::BGSListForm* a_vendorList, RE::MARKER_TYPE a_marker)
{
	vendorMarkers[a_vendorList] = a_marker;
}

auto LocalMapManager::GetSpecialMarkerType(SpecialMarkerData* a_data) -> RE::MARKER_TYPE
{
	auto type = _GetSpecialMarkerType(a_data);

	constexpr RE::MARKER_TYPE kDoor = static_cast<RE::MARKER_TYPE>(61);

	if (type == kDoor) {

		RE::NiPointer<RE::TESObjectREFR> objectRef;
		RE::LookupReferenceByHandle(a_data->refHandle, objectRef);

		auto teleport = objectRef ? objectRef->extraList.GetByType<RE::ExtraTeleport>() : nullptr;

		if (teleport) {

			auto linkedDoor = teleport->teleportData->linkedDoor.get();
			auto location = linkedDoor->GetEditorLocation();
			auto locTypeStore = RE::TESForm::LookupByEditorID<RE::BGSKeyword>("LocTypeStore"sv);

			if (location && location->HasKeyword(locTypeStore)) {

				auto importManager = GetSingleton();
				auto vendorList = importManager->GetVendorList(location);

				if (vendorList) {

					auto it = importManager->vendorMarkers.find(vendorList);
					if (it != importManager->vendorMarkers.end()) {
						return it->second;
					}
				}
			}
		}
	}

	return type;
}

auto LocalMapManager::GetVendorList(RE::BGSLocation* a_location) -> RE::BGSListForm*
{
	auto it = vendorLists.find(a_location);
	return it != vendorLists.end() ? it->second : nullptr;
}
