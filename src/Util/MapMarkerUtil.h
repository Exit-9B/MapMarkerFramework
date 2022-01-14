#pragma once

namespace Util
{
	auto MakeReplaceObject(RE::GFxMovieDataDef* a_movieData, std::uint16_t a_characterId)
		-> RE::GFxPlaceObjectBase*;

	auto MakeRemoveObject(RE::GFxMovieDataDef* a_movieData) -> RE::GFxRemoveObject2*;

	auto MakeMarkerFrameAction(RE::GFxMovieDataDef* a_movieData, float a_iconScale = 1.0f)
		-> RE::GASDoAction*;
}
