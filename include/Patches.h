#pragma once

namespace Patch
{
	using AssignMusicCallback = auto(RE::BSFixedString*, RE::MARKER_TYPE) -> RE::BSFixedString*;

	bool WriteDiscoveryMusicPatch(AssignMusicCallback* a_callback);
}
