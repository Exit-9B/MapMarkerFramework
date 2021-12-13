#pragma once

#include "ImportData.h"

class ImportManager
{
public:
	static auto GetSingleton() -> ImportManager*;

	static void InstallHooks();

	~ImportManager() = default;
	ImportManager(const ImportManager& other) = delete;
	ImportManager(ImportManager&& other) = delete;
	ImportManager& operator=(const ImportManager& other) = delete;
	ImportManager& operator=(ImportManager&& other) = delete;

	void AddCustomIcon(
		std::filesystem::path a_source,
		const std::string& a_exportName,
		const std::string& a_exportNameUndiscovered,
		float a_iconScale,
		bool a_hideFromHUD);

	void HideFromHUD(RE::MARKER_TYPE a_markerType);

private:
	ImportManager() = default;

	void SetupHUDMenu(RE::GFxMovieView* a_movieView);

	void SetupMapMenu(RE::GFxMovieView* a_movieView);

	static void RemoveFrame(
		RE::GFxMovieDataDef* a_movieDataDef,
		RE::GFxSpriteDef* a_marker,
		std::uint32_t a_frame);

	static void FixDoorMarker(
		RE::GFxMovieDataDef* a_movieDataDef,
		RE::GFxSpriteDef* a_marker,
		std::uint32_t a_frame);

	static bool LoadMovie(
		RE::BSScaleformManager* a_scaleformManager,
		RE::IMenu* a_menu,
		RE::GPtr<RE::GFxMovieView>& a_movieView,
		const char* a_fileName,
		RE::GFxMovieView::ScaleModeType a_scaleMode,
		float a_backgroundAlpha);

	std::vector<IconInfo> _customIcons;
	std::uint32_t _baseIndex = 0;

	std::unordered_set<RE::MARKER_TYPE> _hideFromHUD;

	inline static REL::Relocation<decltype(LoadMovie)> _LoadMovie;
};
