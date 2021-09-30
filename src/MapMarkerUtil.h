#pragma once

namespace MapMarkerUtil
{
	void LoadAllMapMarkers();
	void LoadMapMarkersFromFile(std::filesystem::path a_file);

	void UpdateMapMarker(RE::TESObjectREFR* a_markerRef, std::uint8_t a_icon);
};
