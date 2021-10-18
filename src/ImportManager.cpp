#include "ImportManager.h"

auto ImportManager::GetSingleton() -> ImportManager*
{
	static ImportManager singleton{};
	return std::addressof(singleton);
}

auto ImportManager::AddCustomIcon(std::filesystem::path a_source, const std::string& a_exportName)
	-> std::uint32_t
{
	_customIcons.push_back({ a_source, a_exportName });

	auto canonical = std::filesystem::canonical(a_source).string();

	//if (!_importedMovies.contains(canonical)) {
	//	RE::GFxLoader loader{};
	//	auto movieDef = loader.CreateMovie(canonical.c_str());
	//	movieDef->AddRef();

	//	_importedMovies[canonical] = movieDef;
	//}

	return _nextIndex++;
}
