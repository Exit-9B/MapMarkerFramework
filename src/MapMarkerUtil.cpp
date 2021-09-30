#include "MapMarkerUtil.h"
#include "FormUtil.h"
#include <json/json.h>

void MapMarkerUtil::LoadAllMapMarkers()
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
			LoadMapMarkersFromFile(mapMarkerFile);
		}
	}
}

void MapMarkerUtil::LoadMapMarkersFromFile(std::filesystem::path a_file)
{
	Json::Value root;
	if (auto fs = std::ifstream{ a_file }) {
		fs >> root;
	}

	Json::Value mapMarkers = root["mapMarkers"];
	if (!mapMarkers.isArray()) {
		return;
	}

	for (auto& mapMarker : mapMarkers) {
		if (!mapMarker.isObject()) {
			return;
		}

		Json::Value refID = mapMarker["refID"];
		Json::Value icon = mapMarker["icon"];

		if (!refID.isString() || !icon.isUInt()) {
			return;
		}

		auto markerRef = skyrim_cast<RE::TESObjectREFR*>(
			FormUtil::GetFormFromIdentifier(refID.asString()));

		if (markerRef) {
			UpdateMapMarker(markerRef, static_cast<std::uint8_t>(icon.asUInt()));
		}
	}
}

void MapMarkerUtil::UpdateMapMarker(RE::TESObjectREFR* a_markerRef, std::uint8_t a_icon)
{
	auto extraMapMarker = a_markerRef->extraList.GetByType<RE::ExtraMapMarker>();
	if (extraMapMarker) {
		extraMapMarker->mapData->type = static_cast<RE::MARKER_TYPE>(a_icon);
	}
}
