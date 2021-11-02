#include "MapConfigLoader.h"

#include "DiscoveryMusicManager.h"
#include "FormUtil.h"
#include "ImportManager.h"
#include "Settings.h"
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

	for (auto file : dataHandler->files) {
		if (!file) {
			continue;
		}

		auto fileName = std::filesystem::path{ file->fileName };
		fileName.replace_extension("json"sv);
		auto mapMarkerFile = std::filesystem::path{ "MapMarkers" } / fileName;

		RE::BSResourceNiBinaryStream fileStream{ mapMarkerFile.string() };

		if (!fileStream.good()) {
			continue;
		}

		LoadFromFile(file->fileName, fileStream);
	}
}

RE::BSResourceNiBinaryStream& operator>>(RE::BSResourceNiBinaryStream& a_sin, Json::Value& a_root)
{
	Json::CharReaderBuilder fact;
	std::unique_ptr<Json::CharReader> const reader{ fact.newCharReader() };

	auto size = a_sin.stream->totalSize;
	auto buffer = std::make_unique<char[]>(size);
	a_sin.read(buffer.get(), size);

	auto begin = buffer.get();
	auto end = begin + size;

	std::string errs;
	bool ok = reader->parse(begin, end, std::addressof(a_root), std::addressof(errs));

	if (!ok) {
		throw std::runtime_error{ errs };
	}

	return a_sin;
}

void MapConfigLoader::LoadFromFile(
	const std::string& a_fileName,
	RE::BSResourceNiBinaryStream& a_fileStream)
{
	Json::Value root;
	a_fileStream >> root;

	auto importManager = ImportManager::GetSingleton();
	auto discoveryMusicManager = DiscoveryMusicManager::GetSingleton();

	Json::Value iconDefinitions = root["iconDefinitions"];
	if (iconDefinitions.isArray()) {
		for (auto& iconDef : iconDefinitions) {
			if (!iconDef.isObject()) {
				logger::warn(
					"Failed to fetch icon definitions from {}"sv,
					a_fileName);
				continue;
			}

			auto name = iconDef["name"].asString();
			auto& source = iconDef["source"];

			auto scale = iconDef["scale"].asFloat();
			if (!scale) {
				scale = 1.0f;
			}

			auto discoveryMusic = iconDef["discoveryMusic"].asString();

			if (!name.empty() && source.isObject()) {
				auto path = source["path"].asString();

				if (path.empty()) {
					path = Settings::GetSingleton()->Resources.sResourceFile;
				}

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
				logger::warn("Failed to fetch map markers from {}"sv, a_fileName);
				continue;
			}

			auto refID = mapMarker["refID"].asString();

			if (refID.empty()) {
				logger::warn(
					"Map marker missing reference ID in {}"sv,
					a_fileName);
				continue;
			}

			auto markerRef = skyrim_cast<RE::TESObjectREFR*>(
				FormUtil::GetFormFromIdentifier(refID));

			if (!markerRef) {
				logger::warn(
					"'{}' did not correspond to a valid Object Reference in {}"sv,
					refID,
					a_fileName);
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
				logger::warn("Map marker missing icon data in {}"sv, a_fileName);
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
