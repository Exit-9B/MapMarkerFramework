#pragma once

namespace MapMarkerUtil
{
	using AllocateCallback = std::function<void*(std::size_t)>;

	auto MakeReplaceObject(AllocateCallback a_alloc, std::uint16_t a_characterId)
		-> RE::GFxPlaceObjectBase*;

	auto MakeRemoveObject(AllocateCallback a_alloc) -> RE::GFxRemoveObject2*;

	auto MakeMarkerFrameAction(AllocateCallback a_alloc, float a_iconScale = 1.0f)
		-> RE::GASDoAction*;
}
