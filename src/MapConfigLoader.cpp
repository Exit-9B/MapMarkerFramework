#include "MapConfigLoader.h"

#include "DiscoveryMusicManager.h"
#include "FormUtil.h"
#include "ImportManager.h"
#include <json/json.h>

void MapConfigLoader::LoadAll()
{
	const auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (!dataHandler) {
		return;
	}

	auto mapMarkerDir = std::filesystem::path{ "Data\\MapMarkers" };
	for (auto file : dataHandler->files) {
		if (!file) {
			continue;
		}

		auto fileName = std::filesystem::path{ file->fileName };
		fileName.replace_extension("json"sv);
		auto mapMarkerFile = mapMarkerDir / fileName;

		auto dirEntry = std::filesystem::directory_entry{ mapMarkerFile };
		if (dirEntry.exists()) {
			LoadFromFile(mapMarkerFile);
		}
	}

	UpdateMarkers();
}

void MapConfigLoader::LoadFromFile(std::filesystem::path a_file)
{
	Json::Value root;
	if (auto fs = std::ifstream{ a_file }) {
		fs >> root;
	}

	auto importManager = ImportManager::GetSingleton();
	auto discoveryMusicManager = DiscoveryMusicManager::GetSingleton();

	Json::Value iconDefinitions = root["iconDefinitions"];
	if (iconDefinitions.isArray()) {
		for (auto& iconDef : iconDefinitions) {
			if (!iconDef.isObject()) {
				logger::warn(
					"Failed to fetch icon definitions from {}"sv,
					a_file.filename().string());
				continue;
			}

			auto name = iconDef["name"].asString();
			auto source = iconDef["source"].asString();
			auto exportName = iconDef["exportName"].asString();
			auto discoveryMusic = iconDef["discoveryMusic"].asString();

			std::uint32_t index = 0;

			if (!name.empty() && !source.empty()) {
				index = importManager->AddCustomIcon(source, exportName);
				_iconNames[name] = static_cast<RE::MARKER_TYPE>(index);
			}
			else {
				// TODO enforce max index from swf
				index = iconDef["index"].asUInt();
			}

			auto markerType = static_cast<RE::MARKER_TYPE>(index);
			discoveryMusicManager->SetMusic(markerType, discoveryMusic);
		}
	}

	Json::Value mapMarkers = root["mapMarkers"];
	if (mapMarkers.isArray()) {
		for (auto& mapMarker : mapMarkers) {
			if (!mapMarker.isObject()) {
				logger::warn("Failed to fetch map markers from {}"sv, a_file.filename().string());
				continue;
			}

			auto refID = mapMarker["refID"].asString();

			if (refID.empty()) {
				logger::warn(
					"Map marker missing reference ID in {}"sv,
					a_file.filename().string());
				continue;
			}

			auto markerRef = skyrim_cast<RE::TESObjectREFR*>(
				FormUtil::GetFormFromIdentifier(refID));

			if (!markerRef) {
				logger::warn(
					"'{}' did not correspond to a valid Object Reference in {}"sv,
					refID,
					a_file.filename().string());
				continue;
			}

			auto icon = mapMarker["icon"].asUInt();
			auto markerType = static_cast<RE::MARKER_TYPE>(icon);

			auto iconName = mapMarker["iconName"].asString();

			if (!iconName.empty()) {
				_mapMarkers[markerRef] = iconName;
			}
			else if (markerType != RE::MARKER_TYPE::kNone) {
				_mapMarkers[markerRef] = markerType;
			}
			else {
				logger::warn("Map marker missing icon data in {}"sv, a_file.filename().string());
			}
		}
	}
}

void MapConfigLoader::UpdateMarkers()
{
	for (auto& [markerRef, marker] : _mapMarkers) {
		auto markerType = ResolveMarker(marker);
		if (!markerRef || markerType == RE::MARKER_TYPE::kNone) {
			continue;
		}

		UpdateMapMarker(markerRef, markerType);
	}
}

auto MapConfigLoader::ResolveMarker(MapMarker a_marker) -> RE::MARKER_TYPE
{
	if (std::holds_alternative<RE::MARKER_TYPE>(a_marker)) {
		return std::get<RE::MARKER_TYPE>(a_marker);
	}
	else if (std::holds_alternative<std::string>(a_marker)) {
		auto item = _iconNames.find(std::get<std::string>(a_marker));
		if (item != _iconNames.end()) {
			return item->second;
		}
	}

	return RE::MARKER_TYPE::kNone;
}

void MapConfigLoader::UpdateMapMarker(RE::TESObjectREFR* a_markerRef, RE::MARKER_TYPE a_icon)
{
	auto extraMapMarker = a_markerRef->extraList.GetByType<RE::ExtraMapMarker>();
	if (extraMapMarker) {
		extraMapMarker->mapData->type = a_icon;
	}
}
