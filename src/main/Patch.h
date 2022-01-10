#pragma once

namespace Patch
{
	using AssignMusicCallback = auto(RE::BSFixedString*, RE::MARKER_TYPE) -> RE::BSFixedString*;

	bool WriteDiscoveryMusicPatch(AssignMusicCallback* a_callback);

	using LoadMovieFunc =
		bool(
			RE::BSScaleformManager*,
			RE::IMenu*,
			RE::GPtr<RE::GFxMovieView>&,
			const char*,
			RE::GFxMovieView::ScaleModeType,
			float);

	bool WriteLoadHUDPatch(LoadMovieFunc* a_newCall, REL::Relocation<LoadMovieFunc>& a_origCall);

	bool WriteLoadMapPatch(LoadMovieFunc* a_newCall, REL::Relocation<LoadMovieFunc>& a_origCall);

	using GetSpecialMarkerFunc = auto(SpecialMarkerData*) -> RE::MARKER_TYPE;

	bool WriteLocalMapPatch(
		GetSpecialMarkerFunc* a_newCall,
		REL::Relocation<GetSpecialMarkerFunc>& a_origCall);
}
