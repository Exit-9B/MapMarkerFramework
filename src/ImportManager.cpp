#include "ImportManager.h"
#include "MapConfigLoader.h"
#include "GFxUtil.h"
#include "Settings.h"

namespace chrono = std::chrono;

auto ImportManager::GetSingleton() -> ImportManager*
{
	static ImportManager singleton{};
	return std::addressof(singleton);
}

void ImportManager::AddCustomIcon(
	std::filesystem::path a_source,
	const std::string& a_exportName,
	const std::string& a_exportNameUndiscovered,
	float a_iconScale)
{
	auto path = std::filesystem::path{ "MapMarkers"sv } / a_source;
	auto pathStr = path.string();

	_customIcons.push_back({ pathStr, a_exportName, a_exportNameUndiscovered, a_iconScale });
}

void ImportManager::InstallHooks()
{
	auto& trampoline = SKSE::GetTrampoline();

	std::uintptr_t hud_hook = Offset::HUDMenu::Ctor.address() + 0xFE;
	_LoadMovie = trampoline.write_call<5>(hud_hook, LoadMovie);

	std::uintptr_t map_hook = Offset::MapMenu::Ctor.address() + 0x1D1;
	trampoline.write_call<5>(map_hook, LoadMovie);

	logger::info("Installed hooks for movie setup"sv);
}

void ImportManager::SetupHUDMenu(RE::GFxMovieView* a_movieView)
{
	assert(a_movieView);

	auto movieDef = a_movieView->GetMovieDef();
	auto resource = movieDef ? movieDef->GetResource("Compass Marker") : nullptr;

	auto resourceType =
		resource ? resource->GetResourceType() : RE::GFxResource::ResourceType::kNone;

	auto compassMarker =
		resourceType == RE::GFxResource::ResourceType::kSpriteDef
		? static_cast<RE::GFxSpriteDef*>(resource)
		: nullptr;

	if (!compassMarker) {
		logger::error("Could not get compass marker sprite"sv);
		return;
	}

	std::int32_t locationMarkers = 0;
	compassMarker->GetLabeledFrame("LocationMarkers", locationMarkers, false);

	std::int32_t undiscoveredMarkers = 0;
	compassMarker->GetLabeledFrame("UndiscoveredMarkers", undiscoveredMarkers, false);

	std::size_t undiscoveredOffset = static_cast<size_t>(undiscoveredMarkers) - locationMarkers;
	std::size_t insertPos = static_cast<size_t>(undiscoveredMarkers) - 1;

	// If using a modded HUD, walk back to find the RemoveObject frame
	// Vanilla has 60-66 reserved
	std::size_t reservedEnd = static_cast<size_t>(locationMarkers + 66);
	while (insertPos > reservedEnd && !compassMarker->frames[insertPos].data) {
		insertPos--;
	}

	logger::trace(
		"Inserting icons at position {} + undiscovered offset {}"sv,
		insertPos,
		undiscoveredOffset);

	std::size_t iconCount = insertPos - locationMarkers + 1;

	assert(iconCount >= 67);

	// HUD should only load once, but guard here just in case
	if (!_baseIndex) {
		_baseIndex = static_cast<std::uint32_t>(iconCount);
		MapConfigLoader::GetSingleton()->UpdateMarkers(_baseIndex);
	}

	auto movieDefImpl = GFxUtil::GetMovieDefImpl(movieDef);
	if (!movieDefImpl) {
		return;
	}

	if (!_customIcons.empty()) {
		ImportData importData{
			movieDefImpl,
			compassMarker,
			MenuType::HUD,
		};

		std::int32_t newFrames = static_cast<std::int32_t>(_customIcons.size());
		std::int32_t undiscoveredMarkersNew = undiscoveredMarkers + newFrames;

		importData.InsertCustomIcons(
			_customIcons,
			insertPos,
			undiscoveredOffset,
			_baseIndex);

		*(compassMarker->frameLabels.Get("UndiscoveredMarkers")) += newFrames;

		std::int32_t compassMarkerUndiscovered = undiscoveredMarkersNew + 1;

		logger::trace("Updating CompassMarkerUndiscovered to {}"sv, compassMarkerUndiscovered);

		a_movieView->SetVariableDouble(
			"HUDMovieBaseInstance.CompassMarkerUndiscovered",
			compassMarkerUndiscovered);
	}
}

void ImportManager::SetupMapMenu(RE::GFxMovieView* a_movieView)
{
	assert(a_movieView);
	auto movieDef = a_movieView->GetMovieDef();
	class MarkerArtFinder : public RE::GFxMovieDef::ImportVisitor
	{
	public:
		void Visit(
			[[maybe_unused]] RE::GFxMovieDef* a_parentDef,
			RE::GFxMovieDef* a_importDef,
			const char* a_importedMovieFilename) override
		{
			if (_stricmp(a_importedMovieFilename, "skyui\\mapmarkerart.swf") == 0 ||
				_stricmp(a_importedMovieFilename, "CoMAP\\comapart.gfx") == 0) {

				auto resource = a_importDef->GetResource("MapMarkerArt");
				auto resourceType =
					resource ? resource->GetResourceType() : RE::GFxResource::ResourceType::kNone;

				mapMarker =
					resourceType == RE::GFxResource::ResourceType::kSpriteDef
					? static_cast<RE::GFxSpriteDef*>(resource)
					: nullptr;

				movieDef = mapMarker ? a_importDef : nullptr;
			}
		}

		RE::GFxMovieDef* movieDef;
		RE::GFxSpriteDef* mapMarker;
	};
	MarkerArtFinder markerArtFinder{};
	movieDef->VisitImportedMovies(std::addressof(markerArtFinder));

	movieDef = markerArtFinder.movieDef;
	auto mapMarker = markerArtFinder.mapMarker;

	if (!mapMarker) {
		logger::error("SkyUI Map is not installed, can't add map markers"sv);
		return;
	}

	std::int32_t discovered = 0;
	mapMarker->GetLabeledFrame("Discovered", discovered, false);

	std::int32_t undiscovered = 0;
	mapMarker->GetLabeledFrame("Undiscovered", undiscovered, false);

	std::size_t undiscoveredOffset = static_cast<size_t>(undiscovered - discovered);
	std::size_t insertPos = static_cast<size_t>(discovered + _baseIndex);

	if (!_baseIndex) {
		logger::critical("HUD didn't load correctly, can't load Map"sv);
		return;
	}

	logger::trace(
		"Inserting icons at position {} + undiscovered offset {}"sv,
		insertPos,
		undiscoveredOffset);

	auto movieDefImpl = GFxUtil::GetMovieDefImpl(movieDef);
	if (!movieDefImpl) {
		return;
	}

	if (!_customIcons.empty()) {
		ImportData importData{
			movieDefImpl,
			mapMarker,
			MenuType::Map,
		};

		std::int32_t newFrames = static_cast<std::int32_t>(_customIcons.size());
		std::size_t undiscoveredOffsetNew = undiscoveredOffset + newFrames;

		importData.InsertCustomIcons(
			_customIcons,
			insertPos,
			undiscoveredOffset,
			_baseIndex);

		*(mapMarker->frameLabels.Get("Undiscovered")) += newFrames;

		logger::trace("Updating UNDISCOVERED_OFFSET to {}"sv, undiscoveredOffsetNew);

		a_movieView->SetVariableDouble(
			"Map.MapMarker.UNDISCOVERED_OFFSET",
			static_cast<double>(undiscoveredOffsetNew));

		auto typeRangeNew = _baseIndex + newFrames + 1;

		logger::trace("Updating TYPE_RANGE to {}"sv, typeRangeNew);

		a_movieView->SetVariableDouble(
			"Map.LocationFinder.TYPE_RANGE",
			static_cast<double>(typeRangeNew));

		RE::GFxValue iconMap;
		a_movieView->GetVariable(std::addressof(iconMap), "Map.MapMarker.ICON_MAP");
		if (iconMap.IsArray()) {
			logger::trace(
				"ICON_MAP has {} names, adding {} more"sv,
				iconMap.GetArraySize(),
				_customIcons.size());

			for (std::size_t i = 0, size = _customIcons.size(); i < size; i++) {
				// icon names are arbitrary, just make sure they're not recognized by the script
				auto str = fmt::format("Marker{}"sv, i);
				iconMap.PushBack(str.data());
			}
		}
		else {
			logger::error("ICON_MAP was missing or not an array, failed to update"sv);
		}

		auto settings = Settings::GetSingleton();
		if (settings->Map.fMarkerScale != 1.0f) {

			RE::GFxValue markerBaseSize;

			a_movieView->GetVariable(
				std::addressof(markerBaseSize),
				"Map.MapMarker.MARKER_BASE_SIZE");

			auto newSize = markerBaseSize.GetNumber() * settings->Map.fMarkerScale;

			logger::trace("Updating MARKER_BASE_SIZE to {}"sv, typeRangeNew);
			a_movieView->SetVariable("Map.MapMarker.MARKER_BASE_SIZE", newSize);
		}
	}
}

bool ImportManager::LoadMovie(
	RE::BSScaleformManager* a_scaleformManager,
	RE::IMenu* a_menu,
	RE::GPtr<RE::GFxMovieView>& a_movieView,
	const char* a_menuName,
	RE::GFxMovieView::ScaleModeType a_scaleMode,
	float a_backgroundAlpha)
{
	auto result = _LoadMovie(
		a_scaleformManager,
		a_menu,
		a_movieView,
		a_menuName,
		a_scaleMode,
		a_backgroundAlpha);

	if (result) {

		auto startTime = chrono::steady_clock::now();

		if (strcmp(a_menuName, "HUDMenu") == 0) {
			GetSingleton()->SetupHUDMenu(a_movieView.get());
		}
		else if (strcmp(a_menuName, "Map") == 0) {
			GetSingleton()->SetupMapMenu(a_movieView.get());
		}

		auto endTime = chrono::steady_clock::now();
		auto elapsedMs = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);

		logger::info("Set up {} in {} ms"sv, a_menuName, elapsedMs.count());
	}

	return result;
}
