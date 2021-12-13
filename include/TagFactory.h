#pragma once

class TagFactory
{
public:
	using AllocateCallback = std::function<void*(std::size_t)>;

	TagFactory() = delete;

	enum class PlaceFlags2 : std::uint8_t
	{
		kNone = 0,
		kMove = 1 << 0,
		kHasCharacter = 1 << 1,
		kHasMatrix = 1 << 2,
		kHasColorTransform = 1 << 3,
		kHasRatio = 1 << 4,
		kHasName = 1 << 5,
		kHasClipDepth = 1 << 6,
		kHasClipActions = 1 << 7,
	};

	enum class PlaceFlags3 : std::uint8_t
	{
		kNone = 0,
		kHasFilterList = 1 << 0,
		kHasBlendMode = 1 << 1,
		kHasCacheAsBitmap = 1 << 2,
		kHasClassName = 1 << 3,
		kHasImage = 1 << 4,
		kHasVisible = 1 << 5,
		kOpaqueBackground = 1 << 6,
	};

	static auto MakePlaceObject(AllocateCallback a_alloc, const RE::GFxPlaceObjectData& a_data)
		-> RE::GFxPlaceObjectBase*;

	static auto MakeRemoveObject(AllocateCallback a_alloc, std::uint16_t a_depth)
		-> RE::GFxRemoveObject2*;

	static auto MakeRemoveObject(
		AllocateCallback a_alloc,
		std::uint16_t a_characterId,
		std::uint16_t a_depth) -> RE::GFxRemoveObject*;

	static auto MakeInitImportActions(AllocateCallback a_alloc, std::uint32_t a_movieIndex)
		-> RE::GFxInitImportActions*;

	static auto MakeDoAction(AllocateCallback a_alloc, RE::GASActionBufferData* a_data)
		-> RE::GASDoAction*;
};
