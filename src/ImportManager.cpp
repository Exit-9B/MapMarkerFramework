#include "ImportManager.h"
#include "ActionGenerator.h"
#include "MapConfigLoader.h"
#include "TagFactory.h"

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
	double a_iconScale)
{
	auto path = std::filesystem::path{ "Data\\MapMarkers"sv } / a_source;
	auto pathStr = path.string();

	_customIcons.push_back({ pathStr, a_exportName, a_exportNameUndiscovered, a_iconScale });

	auto scaleformManager = RE::BSScaleformManager::GetSingleton();
	auto loader = scaleformManager->loader;
	if (!_importedMovies.contains(pathStr)) {
		auto movieDef = loader->CreateMovie(pathStr.c_str());

		_importedMovies[pathStr] = static_cast<RE::GFxMovieDefImpl*>(movieDef);
	}
}

void ImportManager::LoadIcons()
{
	using ResourceType = RE::GFxResource::ResourceType;

	_icons[IconTypes::Discovered].resize(_customIcons.size());
	_icons[IconTypes::Undiscovered].resize(_customIcons.size());

	for (std::size_t i = 0, size = _customIcons.size(); i < size; i++) {
		auto& iconInfo = _customIcons[i];
		auto& movie = _importedMovies[iconInfo.SourcePath];
		assert(movie);

		auto discoveredIcon = movie->GetResource(iconInfo.ExportName.data());
		auto undiscoveredIcon = movie->GetResource(iconInfo.ExportNameUndiscovered.data());

		if (discoveredIcon && discoveredIcon->GetResourceType() == ResourceType::kSpriteDef) {
			_icons[IconTypes::Discovered][i] = static_cast<RE::GFxSpriteDef*>(discoveredIcon);
		}

		if (undiscoveredIcon && undiscoveredIcon->GetResourceType() == ResourceType::kSpriteDef) {
			_icons[IconTypes::Undiscovered][i] = static_cast<RE::GFxSpriteDef*>(undiscoveredIcon);
		}
	}
}

void ImportManager::InstallHooks()
{
	auto& trampoline = SKSE::GetTrampoline();

	REL::Relocation<std::uintptr_t> hud_hook{ REL::ID(50716), 0xFF };
	_LoadMovie = trampoline.write_call<5>(hud_hook.address(), LoadMovie);

	REL::Relocation<std::uintptr_t> map_hook{ REL::ID(52206), 0x1CF };
	trampoline.write_call<5>(map_hook.address(), LoadMovie);

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

	logger::trace(
		"Inserting icons at position {} + undiscovered offset {}"sv,
		insertPos,
		undiscoveredOffset);

	// walk back until you find the RemoveObject frame
	while (insertPos > locationMarkers && !compassMarker->frames[insertPos].data) {
		insertPos--;
	}

	if (!_baseIndex) {
		_baseIndex = static_cast<std::uint32_t>(insertPos - locationMarkers + 1);
		MapConfigLoader::GetSingleton()->UpdateMarkers(_baseIndex);
	}

	if (!_customIcons.empty()) {
		InsertCustomIcons(
			static_cast<RE::GFxMovieDefImpl*>(movieDef),
			compassMarker,
			insertPos,
			undiscoveredOffset,
			MenuType::HUD);

		std::int32_t newFrames = static_cast<std::int32_t>(_customIcons.size());
		//*(compassMarker->frameLabels.Get("UndiscoveredMarkers")) += newFrames;

		std::int32_t undiscoveredMarkersNew = undiscoveredMarkers + 1 + newFrames;

		logger::trace("Updating CompassMarkerUndiscovered to {}"sv, undiscoveredMarkersNew);

		a_movieView->SetVariableDouble(
			"HUDMovieBaseInstance.CompassMarkerUndiscovered",
			undiscoveredMarkersNew);
	}
}

void ImportManager::SetupMapMenu(RE::GFxMovieView* a_movieView)
{
	assert(a_movieView);
	auto movieDef = a_movieView->GetMovieDef();
	class ImportVisitor : public RE::GFxMovieDef::ImportVisitor
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
	ImportVisitor visitor{};
	movieDef->VisitImportedMovies(std::addressof(visitor));

	movieDef = visitor.movieDef;
	auto mapMarker = visitor.mapMarker;

	if (!mapMarker) {
		logger::error("Could not get map marker sprite"sv);
		return;
	}

	std::int32_t undiscovered = 0;
	mapMarker->GetLabeledFrame("Undiscovered", undiscovered, false);

	std::size_t undiscoveredOffset = static_cast<size_t>(undiscovered);
	std::size_t insertPos = undiscoveredOffset;

	logger::trace(
		"Inserting icons at position {} + undiscovered offset {}"sv,
		insertPos,
		undiscoveredOffset);

	if (insertPos != _baseIndex) {
		logger::error("Map had {} icons, expected {}"sv, insertPos, _baseIndex);
		return;
	}

	if (!_customIcons.empty()) {
		InsertCustomIcons(
			static_cast<RE::GFxMovieDefImpl*>(movieDef),
			mapMarker,
			insertPos,
			undiscoveredOffset,
			MenuType::Map);

		auto newFrames = static_cast<std::int32_t>(_customIcons.size());
		//*(mapMarker->frameLabels.Get("Undiscovered")) += newFrames;

		std::int32_t undiscoveredOffsetNew = undiscovered + newFrames;

		logger::trace("Updating UNDISCOVERED_OFFSET to {}"sv, undiscoveredOffsetNew);

		a_movieView->SetVariableDouble(
			"Map.MapMarker.UNDISCOVERED_OFFSET",
			undiscoveredOffsetNew);

		RE::GFxValue iconMap;
		a_movieView->GetVariable(std::addressof(iconMap), "Map.MapMarker.ICON_MAP");
		if (iconMap.IsArray()) {
			logger::trace("ICON_MAP has {} icons"sv, iconMap.GetArraySize());
			for (std::size_t i = 0, size = _customIcons.size(); i < size; i++) {
				auto str = fmt::format("Marker{}"sv, i);
				iconMap.PushBack(str.data());
			}
		}
	}
}

void ImportManager::InsertCustomIcons(
	RE::GFxMovieDefImpl* a_movieDef,
	RE::GFxSpriteDef* a_marker,
	std::size_t a_insertPos,
	std::size_t a_undiscoveredOffset,
	MenuType a_menuType)
{
	assert(a_marker);

	auto newFrames = static_cast<std::int32_t>(_customIcons.size());

	a_marker->frames.InsertMultipleAt(a_insertPos, newFrames);
	auto undiscoveredOffset = a_undiscoveredOffset + newFrames;

	a_marker->frames.InsertMultipleAt(a_insertPos + undiscoveredOffset, newFrames);

	a_marker->frameCount += newFrames * 2;
	a_marker->frameLoading += newFrames * 2;

	auto& loadTaskData = a_marker->movieData->loadTaskData;
	auto& allocator = loadTaskData->allocator;
	auto alloc = [&allocator](std::size_t a_size) { return allocator.Alloc(a_size); };

	std::unordered_map<std::string, std::uint32_t> movieIndices;
	ImportMovies(a_movieDef, movieIndices);

	std::vector<std::uint16_t> ids[IconTypes::Total];
	std::vector<double> iconScales;
	ImportResources(a_movieDef, movieIndices, ids, iconScales);

	using PlaceFlags = RE::GFxPlaceObject2::PlaceFlags;
	for (std::int32_t iconType = 0; iconType < IconTypes::Total; iconType++) {
		auto insertPos = a_insertPos;
		if (iconType == IconTypes::Undiscovered) {
			insertPos += undiscoveredOffset;
		}

		for (std::int32_t i = 0; i < newFrames; i++) {

			auto placeObject = MakeReplaceObject(alloc, ids[iconType][i]);
			assert(placeObject);

			if (a_menuType == MenuType::Map && iconScales[i] != 1.0) {
				auto doAction = MakeMarkerScaleAction(alloc, iconScales[i]);
				assert(doAction);

				a_marker->frames[i + insertPos] = MakeTagList(alloc, { placeObject, doAction });
			}
			else {
				a_marker->frames[i + insertPos] = MakeTagList(alloc, { placeObject });
			}
		}
	}
}

void ImportManager::ImportMovies(
	RE::GFxMovieDefImpl* a_movieDef,
	std::unordered_map<std::string, std::uint32_t>& a_movieIndices)
{
	auto& bindTaskData = a_movieDef->bindTaskData;
	auto& loadTaskData = bindTaskData->movieDataResource->loadTaskData;

	auto& allocator = loadTaskData->allocator;
	auto alloc = [&allocator](std::size_t a_size) { return allocator.Alloc(a_size); };

	auto& importedMovies = bindTaskData->importedMovies;
	std::size_t numImportedMovies = importedMovies.GetSize();
	importedMovies.Reserve(numImportedMovies + _importedMovies.size());

	for (auto& [path, movie] : _importedMovies) {
		auto movieIndex = importedMovies.GetSize();
		a_movieIndices[path] = static_cast<std::uint32_t>(movieIndex);
		importedMovies.PushBack(movie);
	}

	std::size_t arraySize = sizeof(RE::GASExecuteTag*) * _importedMovies.size();
	auto tagList = static_cast<RE::GASExecuteTag**>(allocator.Alloc(arraySize));
	for (std::size_t i = 0, size = _importedMovies.size(); i < size; i++) {
		auto movie = numImportedMovies + i;
		tagList[i] = TagFactory::MakeInitImportActions(
			alloc,
			static_cast<std::uint32_t>(movie));
	}

	loadTaskData->importFrames.PushBack(ExecuteTagList{
		.data = tagList,
		.size = static_cast<std::uint32_t>(_importedMovies.size()),
	});

	loadTaskData->importFrameCount++;
}

void ImportManager::ImportResources(
	RE::GFxMovieDefImpl* a_movieDef,
	const std::unordered_map<std::string, std::uint32_t>& a_movieIndices,
	std::vector<std::uint16_t> a_ids[],
	std::vector<double>& a_iconScales)
{
	using ImportedResource = RE::GFxMovieDefImpl::ImportedResource;

	auto& bindTaskData = a_movieDef->bindTaskData;
	auto& importData = bindTaskData->importData;

	auto& loadTaskData = bindTaskData->movieDataResource->loadTaskData;
	auto& resources = loadTaskData->resources;

	auto& allocator = loadTaskData->allocator;

	std::uint16_t nextId = 0;
	std::vector<ImportedResource> newResources;

	for (std::size_t iconIndex = 0, size = _customIcons.size(); iconIndex < size; iconIndex++) {
		auto& icon = _customIcons[iconIndex];
		auto& movie = _importedMovies[icon.SourcePath];
		std::uint32_t movieIndex = a_movieIndices.at(icon.SourcePath);

		auto importInfo = new (allocator.Alloc(sizeof(RE::GFxImportNode))) RE::GFxImportNode{
			.filename = icon.SourcePath.data(),
			.frame = static_cast<std::uint32_t>(loadTaskData->importFrames.GetSize()),
			.movieIndex = movieIndex,
		};

		for (std::int32_t iconType = 0; iconType < IconTypes::Total; iconType++) {
			RE::GFxResourceID resourceId;
			do {
				resourceId = RE::GFxResourceID{ ++nextId };
			} while (resources.Find(resourceId) != resources.end());

			a_ids[iconType].push_back(nextId);

			RE::GFxResourceSource resourceSource{};
			resourceSource.type = RE::GFxResourceSource::kImported;
			resourceSource.data.importSource.index = loadTaskData->importedResourceCount;

			loadTaskData->importedResourceCount++;
			loadTaskData->resources.Add(resourceId, resourceSource);

			RE::GFxImportNode::ImportAssetInfo assetInfo{};
			assetInfo.name =
				iconType == IconTypes::Discovered ? icon.ExportName
				: icon.ExportNameUndiscovered;

			assetInfo.id = nextId;
			assetInfo.importIndex = resourceSource.data.importSource.index;
			importInfo->assets.PushBack(assetInfo);
		}

		a_iconScales.push_back(icon.IconScale);

		if (loadTaskData->importInfoBegin) {
			auto& prev = loadTaskData->importInfoEnd;
			prev->nextInChain = importInfo;
		}
		else {
			loadTaskData->importInfoBegin = importInfo;
		}

		loadTaskData->importInfoEnd = importInfo;

		for (std::int32_t iconType = 0; iconType < IconTypes::Total; iconType++) {
			ImportedResource importedResource{};
			importedResource.importData = std::addressof(movie->bindTaskData->importData);
			importedResource.resource = RE::GPtr(_icons[iconType][iconIndex]);
			newResources.push_back(importedResource);
		}
	}

	std::uint32_t newCount = (loadTaskData->importedResourceCount + 0xF) & -0x10;

	if (newCount > importData.importCount) {
		std::size_t newSize = newCount * sizeof(ImportedResource);
		auto newArray = new (importData.heap->Alloc(newSize, 0)) ImportedResource[newCount];

		for (std::uint32_t i = 0; i < importData.importCount; i++) {
			newArray[i] = importData.resourceArray[i];
		}
		for (std::uint32_t i = 0; i < importData.importCount; i++) {
			importData.resourceArray[i].resource->Release();
		}
		importData.resourceArray = newArray;
		importData.importCount = newCount;
	}

	std::uint32_t start =
		loadTaskData->importedResourceCount - static_cast<std::uint32_t>(newResources.size());

	for (int i = 0; i < newResources.size(); i++) {
		importData.resourceArray[start + i] = newResources[i];
	}
}

auto ImportManager::MakeReplaceObject(AllocateCallback a_alloc, std::uint16_t a_characterId)
	-> RE::GFxPlaceObjectBase*
{
	RE::GFxPlaceObjectData placeObjectData{};
	placeObjectData.placeFlags.set(
		RE::GFxPlaceFlags::kMove,
		RE::GFxPlaceFlags::kHasCharacter,
		RE::GFxPlaceFlags::kHasMatrix);
	placeObjectData.depth = 1;
	placeObjectData.characterId = RE::GFxResourceID{ a_characterId };
	placeObjectData.matrix.SetMatrix(0.8f, 0.0f, 0.0f, 0.8f, 0.0f, 0.0f);

	return TagFactory::MakePlaceObject(a_alloc, placeObjectData);
}

auto ImportManager::MakeMarkerScaleAction(AllocateCallback a_alloc, double a_iconScale)
	-> RE::GASDoAction*
{
	struct Action : ActionGenerator
	{
		Action(double a_iconScale)
		{
			// var marker = this._parent._parent._parent;
			Push("marker");
			Push("this");
			GetVariable();
			Push("_parent");
			GetMember();
			Push("_parent");
			GetMember();
			Push("_parent");
			GetMember();
			DefineLocal();

			// marker._width *= a_iconScale;
			Push("marker");
			GetVariable();
			Push("_width");
			Push("marker");
			GetVariable();
			Push("_width");
			GetMember();
			Push(a_iconScale);
			Multiply();
			SetMember();

			// marker._height *= a_iconScale;
			Push("marker");
			GetVariable();
			Push("_height");
			Push("marker");
			GetVariable();
			Push("_height");
			GetMember();
			Push(a_iconScale);
			Multiply();
			SetMember();
		}
	};
	Action action{ a_iconScale };
	action.Ready();
	auto bufferData = action.GetCode();

	return TagFactory::MakeDoAction(a_alloc, bufferData);
}

auto ImportManager::MakeTagList(
	AllocateCallback a_alloc,
	std::initializer_list<RE::GASExecuteTag*> a_tags) -> ExecuteTagList
{
	std::size_t size = sizeof(RE::GASExecuteTag*) * a_tags.size();
	auto tagArray = static_cast<RE::GASExecuteTag**>(a_alloc(size));

	std::copy(a_tags.begin(), a_tags.end(), tagArray);

	return ExecuteTagList{
		.data = tagArray,
		.size = static_cast<std::uint32_t>(a_tags.size()),
	};
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

		logger::info("Injected custom icons into {} in {} ms"sv, a_menuName, elapsedMs.count());
	}

	return result;
}
