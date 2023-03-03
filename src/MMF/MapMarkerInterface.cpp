#include "MapMarkerInterface.h"
#include "main/ImportManager.h"
#include "main/LocalMapManager.h"
#include "main/MapConfigLoader.h"

namespace MMF::Impl
{
	void MapMarkerInterface::Dispatch()
	{
		SKSE::GetMessagingInterface()
			->Dispatch(kMapMarkerInterface, Get(), sizeof(detail::MapMarkerInterface), nullptr);
	}

	detail::MapMarkerInterface* MapMarkerInterface::Get()
	{
		static detail::MapMarkerInterface intfc{
			.interfaceVersion = InterfaceVersion,
			.GetCustomMarkerName = &GetCustomMarkerName,
			.GetMapMarkerInfo = &GetMapMarkerInfo,
			.GetLocalMarkerInfo = &GetLocalMarkerInfo,
		};

		return std::addressof(intfc);
	}

	std::uint16_t MapMarkerInterface::GetVanillaMarkerType(RE::TESObjectREFR* a_refr)
	{
		if (!a_refr) {
			return 0;
		}

		if (auto extraMapMarker = a_refr->extraList.GetByType<RE::ExtraMapMarker>()) {
			if (auto data = extraMapMarker->mapData) {
				return data->type.underlying();
			}
		}

		return 0;
	}

	const char* MapMarkerInterface::GetCustomMarkerName(RE::TESObjectREFR* a_refr)
	{
		const auto configLoader = MapConfigLoader::GetSingleton();
		const auto& mapMarker = configLoader->GetMapMarker(a_refr);
		if (std::holds_alternative<std::string>(mapMarker)) {
			return std::get<std::string>(mapMarker).data();
		}

		return "";
	}

	void MapMarkerInterface::GetMapMarkerInfo(
		RE::TESObjectREFR* a_refr,
		detail::MARKER_INFO* a_infoOut)
	{
		if (!a_infoOut) {
			return;
		}

		a_infoOut->VanillaMarker = GetVanillaMarkerType(a_refr);
		a_infoOut->SourcePath = nullptr;
		a_infoOut->ExportName = nullptr;
		a_infoOut->ExportNameUndiscovered = nullptr;
		a_infoOut->IconScale = 1.0f;
		a_infoOut->HideFromHUD = false;

		const auto configLoader = MapConfigLoader::GetSingleton();
		const auto importManager = ImportManager::GetSingleton();

		const auto& mapMarker = configLoader->GetMapMarker(a_refr);
		if (std::holds_alternative<RE::MARKER_TYPE>(mapMarker)) {
			a_infoOut->VanillaMarker = static_cast<std::int32_t>(
				std::get<RE::MARKER_TYPE>(mapMarker));
		}
		else if (std::holds_alternative<std::string>(mapMarker)) {
			const auto index = configLoader->GetIconIndex(std::get<std::string>(mapMarker));
			const auto iconInfo = importManager->GetIconInfo(index);
			SetIconInfo(iconInfo, a_infoOut);
		}
	}

	void MapMarkerInterface::GetLocalMarkerInfo(
		RE::BGSLocation* a_location,
		detail::MARKER_INFO* a_infoOut)
	{
		if (!a_infoOut) {
			return;
		}

		a_infoOut->VanillaMarker = static_cast<std::int32_t>(RE::MARKER_TYPE::kDoor);
		a_infoOut->SourcePath = nullptr;
		a_infoOut->ExportName = nullptr;
		a_infoOut->ExportNameUndiscovered = nullptr;
		a_infoOut->IconScale = 1.0f;
		a_infoOut->HideFromHUD = false;

		const auto configLoader = MapConfigLoader::GetSingleton();
		const auto importManager = ImportManager::GetSingleton();

		const auto& mapMarker = configLoader->GetLocalMarker(a_location);
		if (std::holds_alternative<RE::MARKER_TYPE>(mapMarker)) {
			a_infoOut->VanillaMarker = static_cast<std::int32_t>(
				std::get<RE::MARKER_TYPE>(mapMarker));
		}
		else if (std::holds_alternative<std::string>(mapMarker)) {
			const auto index = configLoader->GetIconIndex(std::get<std::string>(mapMarker));
			const auto iconInfo = importManager->GetIconInfo(index);
			SetIconInfo(iconInfo, a_infoOut);
		}
	}

	void MapMarkerInterface::SetIconInfo(const IconInfo* a_info, detail::MARKER_INFO* a_infoOut)
	{
		assert(a_infoOut);

		if (a_info) {
			a_infoOut->SourcePath = a_info->SourcePath.data();
			a_infoOut->ExportName = a_info->ExportName.data();
			a_infoOut->ExportNameUndiscovered = a_info->ExportNameUndiscovered.data();
			a_infoOut->IconScale = a_info->IconScale;
			a_infoOut->HideFromHUD = a_info->HideFromHUD;
		}
	}
}
