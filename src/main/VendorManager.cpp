#include "VendorManager.h"

VendorManager* VendorManager::GetSingleton()
{
	static VendorManager singleton{};
	return &singleton;
}

void VendorManager::Load()
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

RE::BGSListForm* VendorManager::GetVendorList(RE::BGSLocation* a_location)
{
	auto it = _vendorLists.find(a_location);
	return it != _vendorLists.end() ? it->second : nullptr;
}
