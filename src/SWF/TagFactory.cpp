#include "SWF/TagFactory.h"
#include "SWF/SWFOutputStream.h"

namespace SWF
{
	auto TagFactory::MakePlaceObject(
		RE::GFxMovieDataDef* a_movieData,
		const RE::GFxPlaceObjectData& a_data) -> RE::GFxPlaceObjectBase*
	{
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

		static REL::Relocation<std::uintptr_t> GFxPlaceObject2_vtbl{
			Offset::GFxPlaceObject2::Vtbl
		};
		static REL::Relocation<std::uintptr_t> GFxPlaceObject3_vtbl{
			Offset::GFxPlaceObject2::Vtbl
		};

		SWFOutputStream sos;
		RE::stl::enumeration<PlaceFlags2, std::uint8_t> placeFlags2(PlaceFlags2::kNone);
		RE::stl::enumeration<PlaceFlags3, std::uint8_t> placeFlags3(PlaceFlags3::kNone);

		placeFlags2 |= static_cast<PlaceFlags2>(a_data.placeFlags.underlying() & 0x5F);
		if (a_data.name) {
			placeFlags2 |= PlaceFlags2::kHasName;
		}
		if (a_data.clipActions) {
			placeFlags2 |= PlaceFlags2::kHasClipActions;
		}
		if (a_data.placeFlags.all(RE::GFxPlaceFlags::kHasFilterList)) {
			placeFlags3 |= PlaceFlags3::kHasFilterList;
		}
		if (a_data.placeFlags.all(RE::GFxPlaceFlags::kHasBlendMode)) {
			placeFlags3 |= PlaceFlags3::kHasBlendMode;
		}

		sos.WriteUI8(placeFlags2.underlying());
		if (placeFlags3 != PlaceFlags3::kNone) {
			sos.WriteUI8(placeFlags3.underlying());
		}

		sos.WriteUI16(static_cast<std::uint16_t>(a_data.depth));

		if (placeFlags2.all(PlaceFlags2::kHasCharacter)) {
			sos.WriteUI16(a_data.characterId.GetIDIndex() & 0xFFFF);
		}
		if (placeFlags2.all(PlaceFlags2::kHasMatrix)) {
			sos.WriteMATRIX(a_data.matrix);
		}
		if (placeFlags2.all(PlaceFlags2::kHasColorTransform)) {
			sos.WriteCXFORMWITHALPHA(a_data.colorTransform);
		}
		if (placeFlags2.all(PlaceFlags2::kHasRatio)) {
			sos.WriteFIXED8(a_data.ratio);
		}
		if (placeFlags2.all(PlaceFlags2::kHasName)) {
			sos.WriteSTRING(a_data.name);
		}
		if (placeFlags2.all(PlaceFlags2::kHasClipDepth)) {
			sos.WriteUI16(a_data.clipDepth);
		}
		if (placeFlags3.all(PlaceFlags3::kHasFilterList)) {
			sos.WriteFILTERLIST(a_data.filterList);
		}
		if (placeFlags3.all(PlaceFlags3::kHasBlendMode)) {
			sos.WriteUI8(a_data.blendMode.underlying());
		}
		if (placeFlags2.all(PlaceFlags2::kHasClipActions)) {
			// todo
		}

		auto data = sos.Get();
		if (data.empty()) {
			return nullptr;
		}

		if (placeFlags3 == PlaceFlags3::kNone) {
			auto size = ((sizeof(RE::GFxPlaceObject2) - 1 + data.size()) + 7) & -0x8;
			auto placeObject = static_cast<RE::GFxPlaceObject2*>(
				a_movieData->loadTaskData->allocator.Alloc(size));

			if (placeObject) {
				*reinterpret_cast<std::uintptr_t*>(placeObject) = GFxPlaceObject2_vtbl.get();
				memcpy(placeObject->data, data.data(), data.size());
			}
			return placeObject;
		}
		else {
			auto size = ((sizeof(RE::GFxPlaceObject3) - 1 + data.size()) + 7) & -0x8;
			auto placeObject = static_cast<RE::GFxPlaceObject3*>(
				a_movieData->loadTaskData->allocator.Alloc(size));

			if (placeObject) {
				*reinterpret_cast<std::uintptr_t*>(placeObject) = GFxPlaceObject3_vtbl.get();
				memcpy(placeObject->data, data.data(), data.size());
			}
			return placeObject;
		}
	}

	auto TagFactory::MakeRemoveObject(RE::GFxMovieDataDef* a_movieData, std::uint16_t a_depth)
		-> RE::GFxRemoveObject2*
	{
		static REL::Relocation<std::uintptr_t> GFxRemoveObject2_vtbl{
			Offset::GFxRemoveObject2::Vtbl
		};

		auto removeObject = static_cast<RE::GFxRemoveObject2*>(
			a_movieData->loadTaskData->allocator.Alloc(sizeof(RE::GFxRemoveObject2)));

		std::memset(removeObject, 0, sizeof(RE::GFxRemoveObject2));

		if (removeObject) {
			*reinterpret_cast<std::uintptr_t*>(removeObject) = GFxRemoveObject2_vtbl.get();

			removeObject->depth = a_depth;
		}

		return removeObject;
	}

	auto TagFactory::MakeRemoveObject(
		RE::GFxMovieDataDef* a_movieData,
		std::uint16_t a_characterId,
		std::uint16_t a_depth) -> RE::GFxRemoveObject*
	{
		static REL::Relocation<std::uintptr_t> GFxRemoveObject_vtbl{
			Offset::GFxRemoveObject::Vtbl
		};

		auto removeObject = static_cast<RE::GFxRemoveObject*>(
			a_movieData->loadTaskData->allocator.Alloc(sizeof(RE::GFxRemoveObject)));

		std::memset(removeObject, 0, sizeof(RE::GFxRemoveObject));

		if (removeObject) {
			*reinterpret_cast<std::uintptr_t*>(removeObject) = GFxRemoveObject_vtbl.get();

			removeObject->characterId = a_characterId;
			removeObject->depth = a_depth;
		}

		return removeObject;
	}

	auto TagFactory::MakeInitImportActions(RE::GFxMovieDataDef* a_movieData, std::uint32_t a_movieIndex)
		-> RE::GFxInitImportActions*
	{
		static REL::Relocation<std::uintptr_t> GFxInitImportActions_vtbl{
			Offset::GFxInitImportActions::Vtbl
		};

		auto initImportActions = static_cast<RE::GFxInitImportActions*>(
			a_movieData->loadTaskData->allocator.Alloc(sizeof(RE::GFxInitImportActions)));

		std::memset(initImportActions, 0, sizeof(RE::GFxInitImportActions));

		if (initImportActions) {
			*reinterpret_cast<std::uintptr_t*>(
				initImportActions) = GFxInitImportActions_vtbl.get();

			initImportActions->movieIndex = a_movieIndex;
		}

		return initImportActions;
	}

	auto TagFactory::MakeDoAction(RE::GFxMovieDataDef* a_movieData, RE::GASActionBufferData* a_data)
		-> RE::GASDoAction*
	{
		static REL::Relocation<std::uintptr_t> GASDoAction_vtbl{ Offset::GASDoAction::Vtbl };

		auto doAction = static_cast<RE::GASDoAction*>(
			a_movieData->loadTaskData->allocator.Alloc(sizeof(RE::GASDoAction)));
		std::memset(doAction, 0, sizeof(RE::GASDoAction));
		assert(!doAction->data);

		if (doAction) {
			*reinterpret_cast<std::uintptr_t*>(doAction) = GASDoAction_vtbl.get();

			doAction->data = RE::GPtr(a_data);
		}

		return doAction;
	}
}
