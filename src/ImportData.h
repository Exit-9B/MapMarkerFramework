#pragma once

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

class ImportData
{
public:
	ImportData(RE::GFxMovieDefImpl* a_movie, RE::GFxSpriteDef* a_marker, MenuType a_menuType) :
		_targetMovie{ a_movie },
		_marker{ a_marker },
		_menuType{ a_menuType }
	{}

	void InsertCustomIcons(
		const std::vector<IconInfo>& a_iconInfo,
		std::size_t a_insertPos,
		std::size_t a_undiscoveredOffset);

private:
	using AllocateCallback = std::function<void*(std::size_t)>;
	using ExecuteTagList = RE::GFxTimelineDef::ExecuteTagList;

	void LoadIcons(const std::vector<IconInfo>& a_iconInfo);
	void ImportMovies();
	void ImportResources(const std::vector<IconInfo>& a_iconInfo);

	static auto MakeReplaceObject(
		AllocateCallback a_alloc,
		std::uint16_t a_characterId) -> RE::GFxPlaceObjectBase*;

	static auto MakeMarkerScaleAction(
		AllocateCallback a_alloc,
		float a_iconScale) -> RE::GASDoAction*;

	static auto MakeTagList(
		AllocateCallback a_alloc,
		std::initializer_list<RE::GASExecuteTag*> a_tags) -> ExecuteTagList;

private:
	RE::GFxMovieDefImpl* _targetMovie;
	RE::GFxSpriteDef* _marker;
	MenuType _menuType;

	std::unordered_map<std::string, RE::GFxMovieDefImpl*> _importedMovies;
	std::unordered_map<std::string, std::uint32_t> _movieIndices;
	std::vector<RE::GFxSpriteDef*> _icons[IconTypes::Total];
	std::vector<std::uint16_t> _ids[IconTypes::Total];
	std::vector<float> _iconScales;
};
