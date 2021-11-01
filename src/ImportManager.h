#pragma once
#include <unordered_set>

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
		float a_iconScale);

	void LoadIcons(
		std::unordered_map<std::string, RE::GFxMovieDefImpl*>& a_movies,
		std::vector<RE::GFxSpriteDef*> a_icons[]);

private:
	using AllocateCallback = std::function<void*(std::size_t)>;
	using ExecuteTagList = RE::GFxTimelineDef::ExecuteTagList;

	struct IconTypes
	{
		enum
		{
			Discovered,
			Undiscovered,

			Total
		};
	};

	enum class MenuType
	{
		HUD,
		Map,
	};

	struct IconInfo
	{
		std::string SourcePath;
		std::string ExportName;
		std::string ExportNameUndiscovered;
		float IconScale;
	};

	ImportManager() = default;

	void SetupHUDMenu(RE::GFxMovieView* a_movieView);

	void SetupMapMenu(RE::GFxMovieView* a_movieView);

	void InsertCustomIcons(
		RE::GFxMovieDefImpl* a_movieDef,
		RE::GFxSpriteDef* a_marker,
		std::size_t a_insertPos,
		std::size_t a_undiscoveredOffset,
		MenuType a_menuType);

	void ImportMovies(
		RE::GFxMovieDefImpl* a_movieDef,
		const std::unordered_map<std::string, RE::GFxMovieDefImpl*>& a_movies,
		std::unordered_map<std::string, std::uint32_t>& a_movieIndices);

	void ImportResources(
		RE::GFxMovieDefImpl* a_movieDef,
		const std::unordered_map<std::string, RE::GFxMovieDefImpl*>& a_movies,
		const std::unordered_map<std::string, std::uint32_t>& a_movieIndices,
		const std::vector<RE::GFxSpriteDef*> a_icons[],
		std::vector<std::uint16_t> a_ids[],
		std::vector<float>& a_iconScales);

	static auto MakeReplaceObject(
		AllocateCallback a_alloc,
		std::uint16_t a_characterId) -> RE::GFxPlaceObjectBase*;

	static auto MakeMarkerScaleAction(
		AllocateCallback a_alloc,
		float a_iconScale) -> RE::GASDoAction*;

	static auto MakeTagList(
		AllocateCallback a_alloc,
		std::initializer_list<RE::GASExecuteTag*> a_tags) -> ExecuteTagList;

	static bool LoadMovie(
		RE::BSScaleformManager* a_scaleformManager,
		RE::IMenu* a_menu,
		RE::GPtr<RE::GFxMovieView>& a_movieView,
		const char* a_fileName,
		RE::GFxMovieView::ScaleModeType a_scaleMode,
		float a_backgroundAlpha);

	std::vector<IconInfo> _customIcons;

	std::unordered_set<std::string> _importedMovies;
	std::uint32_t _baseIndex = 0;

	inline static REL::Relocation<decltype(LoadMovie)> _LoadMovie;
};
