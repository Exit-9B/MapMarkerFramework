#include "ImportManager.h"

ImportManager::ImportManager()
{
	RE::GFxLoader::LoaderConfig loaderConfig{
		RE::GFxLoader::kLoadAll,
		RE::GPtr<RE::GFxFileOpenerBase>{ new RE::BSScaleformFileOpener{} },
		RE::GPtr<RE::GFxZlibSupportBase>{ new RE::GFxZlibSupport{} },
		RE::GPtr<RE::GFxJpegSupportBase>{ nullptr },
	};

	_loader = new RE::GFxLoader{ loaderConfig };
}

ImportManager::~ImportManager()
{
	delete _loader;
}

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

	if (!_importedMovies.contains(canonical)) {
		auto movieDef = RE::GPtr<RE::GFxMovieDef>{ _loader->CreateMovie(canonical.c_str()) };

		_importedMovies[canonical] = movieDef;
	}

	return _nextIndex++;
}
