#pragma once

namespace SWF
{
	class TagFactory
	{
	public:
		TagFactory() = delete;

		static auto MakePlaceObject(
			RE::GFxMovieDataDef* a_movieData,
			const RE::GFxPlaceObjectData& a_data) -> RE::GFxPlaceObjectBase*;

		static auto MakeRemoveObject(RE::GFxMovieDataDef* a_movieData, std::uint16_t a_depth)
			-> RE::GFxRemoveObject2*;

		static auto MakeRemoveObject(
			RE::GFxMovieDataDef* a_movieData,
			std::uint16_t a_characterId,
			std::uint16_t a_depth) -> RE::GFxRemoveObject*;

		static auto MakeInitImportActions(
			RE::GFxMovieDataDef* a_movieData,
			std::uint32_t a_movieIndex) -> RE::GFxInitImportActions*;

		static auto MakeDoAction(RE::GFxMovieDataDef* a_movieData, RE::GASActionBufferData* a_data)
			-> RE::GASDoAction*;
	};
}
