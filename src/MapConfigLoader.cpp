#include "MapConfigLoader.h"

#include "DiscoveryMusicManager.h"
#include "FormUtil.h"
#include "ImportManager.h"
#include <json/json.h>

auto MapConfigLoader::GetSingleton() -> MapConfigLoader*
{
	static MapConfigLoader singleton{};
	return std::addressof(singleton);
}

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
			auto& source = iconDef["source"];

			auto scale = iconDef["scale"].asFloat();
			if (!scale) {
				scale = 1.0;
			}

			auto discoveryMusic = iconDef["discoveryMusic"].asString();

			if (!name.empty() && source.isObject()) {
				auto path = source["path"].asString();

				std::string exportName, exportNameUndiscovered;
				auto& exportNames = source["exportNames"];
				if (exportNames.isArray() && exportNames.size() == 2) {
					exportName = exportNames[0].asString();
					exportNameUndiscovered = exportNames[1].asString();
				}

				importManager->AddCustomIcon(
					path,
					exportName,
					exportNameUndiscovered,
					scale);

				_iconNames[name] = _lastIcon++;

				if (!discoveryMusic.empty()) {
					_discoveryMusic[name] = discoveryMusic;
				}
			}
			else {
				std::uint32_t index = iconDef["index"].asUInt();

				if (index) {
					auto markerType = static_cast<RE::MARKER_TYPE>(index);
					discoveryMusicManager->SetMusic(markerType, discoveryMusic);
				}
			}
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

void MapConfigLoader::UpdateMarkers(std::uint32_t a_customIconIndex) const
{
	for (auto& [markerRef, marker] : _mapMarkers) {
		auto markerType = ResolveMarker(marker, a_customIconIndex);
		if (!markerRef || markerType == RE::MARKER_TYPE::kNone) {
			continue;
		}

		UpdateMapMarker(markerRef, markerType);

		if (std::holds_alternative<std::string>(marker)) {
			auto& markerName = std::get<std::string>(marker);
			auto it = _discoveryMusic.find(markerName);
			if (it != _discoveryMusic.end()) {
				DiscoveryMusicManager::GetSingleton()->SetMusic(markerType, it->second);
			}
		}
	}
}

auto MapConfigLoader::ResolveMarker(MapMarker a_marker, std::uint32_t a_customIconIndex) const
	-> RE::MARKER_TYPE
{
	if (std::holds_alternative<RE::MARKER_TYPE>(a_marker)) {
		auto markerType = std::get<RE::MARKER_TYPE>(a_marker);
		if (static_cast<std::uint32_t>(markerType) < a_customIconIndex) {
			return markerType;
		}
	}
	else if (std::holds_alternative<std::string>(a_marker) && a_customIconIndex != 0) {
		auto item = _iconNames.find(std::get<std::string>(a_marker));
		if (item != _iconNames.end()) {
			return static_cast<RE::MARKER_TYPE>(a_customIconIndex + item->second);
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
