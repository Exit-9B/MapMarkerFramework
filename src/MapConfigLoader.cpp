#include "MapConfigLoader.h"

#include "DiscoveryMusicManager.h"
#include "FormUtil.h"
#include "ImportManager.h"
#include "LocalMapManager.h"
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

		LoadFromFile(fileName.string(), fileStream);
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

			auto hideFromHUD = iconDef["hideFromHUD"].asBool();

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
					scale,
					hideFromHUD);

				_iconNames[name] = _lastIcon++;

				if (!discoveryMusic.empty()) {
					_discoveryMusic[name] = discoveryMusic;
				}
			}
			else {
				std::uint32_t index = iconDef["index"].asUInt();

				if (index) {
					auto markerType = static_cast<RE::MARKER_TYPE>(index);

					if (!discoveryMusic.empty()) {
						discoveryMusicManager->SetMusic(markerType, discoveryMusic);
					}

					if (hideFromHUD) {
						importManager->HideFromHUD(markerType);
					}
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

			auto icon = mapMarker["icon"].asUInt();
			auto markerType = static_cast<RE::MARKER_TYPE>(icon);

			auto iconName = mapMarker["iconName"].asString();

			if (markerType == RE::MARKER_TYPE::kNone && iconName.empty()) {
				logger::warn("Map marker missing icon data in {}"sv, a_fileName);
			}

			MapMarker markerKey = markerType;
			if (!iconName.empty()) {
				markerKey = iconName;
			}

			if (!refID.empty()) {
				auto markerRef = skyrim_cast<RE::TESObjectREFR*>(
					FormUtil::GetFormFromIdentifier(refID));

				if (!markerRef) {
					logger::warn(
						"'{}' did not correspond to a valid Object Reference in {}"sv,
						refID,
						a_fileName);

					continue;
				}

				_mapMarkers[markerRef] = markerKey;
			}
		}
	}

	Json::Value localMapMarkers = root["localMapMarkers"];
	if (localMapMarkers.isArray()) {
		for (auto& localMapMarker : localMapMarkers) {
			if (!localMapMarker.isObject()) {
				logger::warn("Failed to fetch local map markers from {}"sv, a_fileName);
				continue;
			}

			auto location = localMapMarker["location"].asString();
			auto vendorList = localMapMarker["vendorList"].asString();
			auto locType = localMapMarker["locType"].asString();

			if (location.empty() && vendorList.empty() && locType.empty()) {
				logger::warn(
					"Local map marker missing location / keyword / vendor list in {}"sv,
					a_fileName);

				continue;
			}

			auto icon = localMapMarker["icon"].asUInt();
			auto markerType = static_cast<RE::MARKER_TYPE>(icon);

			auto iconName = localMapMarker["iconName"].asString();

			if (markerType == RE::MARKER_TYPE::kNone && iconName.empty()) {
				logger::warn("Local map marker missing icon data in {}"sv, a_fileName);
			}

			MapMarker markerKey = markerType;
			if (!iconName.empty()) {
				markerKey = iconName;
			}

			if (!location.empty()) {
				auto locRef = skyrim_cast<RE::BGSLocation*>(
					FormUtil::GetFormFromIdentifier(location));

				if (!locRef) {
					logger::warn(
						"'{}' did not correspond to a valid Location in {}"sv,
						location,
						a_fileName);

					continue;
				}

				_locationMarkers[locRef] = markerKey;
			}
			if (!vendorList.empty()) {
				auto buySellList = skyrim_cast<RE::BGSListForm*>(
					FormUtil::GetFormFromIdentifier(vendorList));

				if (!buySellList) {
					logger::warn(
						"'{}' did not correspond to a valid Form List in {}"sv,
						vendorList,
						a_fileName);

					continue;
				}

				_vendorMarkers[buySellList] = markerKey;
			}
			else if (!locType.empty()) {
				auto locKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(locType);

				if (!locKeyword) {
					logger::warn(
						"'{}' did not correspond to a valid Keyword in {}"sv,
						locType,
						a_fileName);

					continue;
				}

				_locTypeMarkers[locKeyword] = markerKey;
			}
		}
	}
}

void MapConfigLoader::UpdateMarkers(std::uint32_t a_customIconIndex) const
{
	for (auto& [markerRef, marker] : _mapMarkers) {
		if (!markerRef) {
			continue;
		}

		auto markerType = ResolveMarker(marker, a_customIconIndex);

		if (markerType == RE::MARKER_TYPE::kNone) {
			auto location = markerRef->GetEditorLocation();
			auto locationName = location ? location->GetFullName() : "BAD LOCATION";

			logger::warn(
				"MapMarker Reference ({:08X}) in {} failed to resolve custom icon"sv,
				markerRef->GetFormID(),
				locationName);

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

	auto localMapManager = LocalMapManager::GetSingleton();

	for (auto& [location, marker] : _locationMarkers) {
		if (!location) {
			continue;
		}

		auto markerType = ResolveMarker(marker, a_customIconIndex);

		if (markerType == RE::MARKER_TYPE::kNone) {
			logger::warn(
				"Location ({:08X}) failed to resolve custom icon"sv,
				location->GetFormID());

			continue;
		}

		localMapManager->AddLocationMarker(location, markerType);
	}

	for (auto& [vendorList, marker] : _vendorMarkers) {
		if (!vendorList) {
			continue;
		}

		auto markerType = ResolveMarker(marker, a_customIconIndex);

		if (markerType == RE::MARKER_TYPE::kNone) {
			logger::warn(
				"Vendor Form List ({:08X}) failed to resolve custom icon"sv,
				vendorList->GetFormID());

			continue;
		}

		localMapManager->AddVendorMarker(vendorList, markerType);
	}

	for (auto& [locType, marker] : _locTypeMarkers) {
		if (!locType) {
			continue;
		}

		auto markerType = ResolveMarker(marker, a_customIconIndex);

		if (markerType == RE::MARKER_TYPE::kNone) {
			logger::warn(
				"Location Keyword ({:08X}) failed to resolve custom icon"sv,
				locType->GetFormID());

			continue;
		}

		localMapManager->AddLocTypeMarker(locType, markerType);
	}
}

auto MapConfigLoader::ResolveMarker(MapMarker a_marker, std::uint32_t a_customIconIndex) const
	-> RE::MARKER_TYPE
{
	if (std::holds_alternative<RE::MARKER_TYPE>(a_marker)) {
		auto markerType = std::get<RE::MARKER_TYPE>(a_marker);
		auto index = static_cast<std::uint32_t>(markerType);
		if (index < a_customIconIndex) {
			return markerType;
		}
		else {
			logger::warn("Icon index {} is out of range"sv, index);
		}
	}
	else if (std::holds_alternative<std::string>(a_marker) && a_customIconIndex != 0) {
		auto& iconName = std::get<std::string>(a_marker);
		auto item = _iconNames.find(iconName);
		if (item != _iconNames.end()) {
			return static_cast<RE::MARKER_TYPE>(a_customIconIndex + item->second);
		}
		else {
			logger::warn("Icon name '{}' is not defined"sv, iconName);
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
