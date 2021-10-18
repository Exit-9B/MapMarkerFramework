#pragma once

class ImportManager
{
public:
	static auto GetSingleton() -> ImportManager*;

	~ImportManager();
	ImportManager(const ImportManager& other) = delete;
	ImportManager(ImportManager&& other) = delete;
	ImportManager& operator=(const ImportManager& other) = delete;
	ImportManager& operator=(ImportManager&& other) = delete;

	// TODO inject the icons

	auto AddCustomIcon(
		std::filesystem::path a_source,
		const std::string& a_exportName) -> std::uint32_t;

private:
	struct IconInfo
	{
		std::filesystem::path SourcePath;
		std::string ExportName;
	};

	ImportManager();

	std::uint32_t _lastIndexFromSwf{ 66 };
	std::uint32_t _nextIndex{ 67 };

	std::vector<IconInfo> _customIcons;

	std::unordered_map<std::string, RE::GPtr<RE::GFxMovieDef>> _importedMovies;

	RE::GFxLoader* _loader;
};
