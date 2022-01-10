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
	bool HideFromHUD;
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
		std::size_t a_undiscoveredOffset,
		std::size_t a_baseCount);

private:
	using AllocateCallback = std::function<void*(std::size_t)>;
	using ExecuteTagList = RE::GFxTimelineDef::ExecuteTagList;

	auto LoadMovie(const std::string& a_sourcePath) -> RE::GFxMovieDefImpl*;

	void LoadIcons();
	void ImportMovies();
	void ImportResources();

private:
	struct IconId
	{
		std::string exportName;
		std::int32_t iconType;
		std::size_t index;
	};

	RE::GFxMovieDefImpl* _targetMovie;
	RE::GFxSpriteDef* _marker;
	MenuType _menuType;

	std::unordered_map<std::string, std::vector<IconId>> _importResources;
	std::size_t _numIcons;

	std::unordered_map<std::string, RE::GFxMovieDefImpl*> _importedMovies;
	std::unordered_map<std::string, std::uint32_t> _movieIndices;
	std::vector<RE::GFxSpriteDef*> _icons[IconTypes::Total];
	std::vector<std::uint16_t> _ids[IconTypes::Total];
	std::vector<float> _iconScales;
	std::vector<bool> _hideHUD;
};
