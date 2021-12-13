#include "MapMarkerUtil.h"
#include "ActionGenerator.h"
#include "TagFactory.h"

auto MapMarkerUtil::MakeReplaceObject(AllocateCallback a_alloc, std::uint16_t a_characterId)
	-> RE::GFxPlaceObjectBase*
{
	RE::GFxPlaceObjectData placeObjectData{};
	placeObjectData.placeFlags.set(
		RE::GFxPlaceFlags::kMove,
		RE::GFxPlaceFlags::kHasCharacter,
		RE::GFxPlaceFlags::kHasMatrix);
	placeObjectData.depth = 1;
	placeObjectData.characterId = RE::GFxResourceID{ a_characterId };
	placeObjectData.matrix.SetMatrix(0.8f, 0.0f, 0.0f, 0.8f, 0.0f, 0.0f);

	return TagFactory::MakePlaceObject(a_alloc, placeObjectData);
}

auto MapMarkerUtil::MakeRemoveObject(AllocateCallback a_alloc) -> RE::GFxRemoveObject2*
{
	return TagFactory::MakeRemoveObject(a_alloc, 1);
}

auto MapMarkerUtil::MakeMarkerFrameAction(AllocateCallback a_alloc, float a_iconScale)
	-> RE::GASDoAction*
{
	struct Action : ActionGenerator
	{
		Action(float a_iconScale)
		{
			Label endLbl;
			Label doorLbl;

			// var marker = this._parent._parent._parent;
			Push("marker");
			Push("this");
			GetVariable();
			Push("_parent");
			GetMember();
			Push("_parent");
			GetMember();
			Push("_parent");
			GetMember();
			DefineLocal();

			// if (marker instanceof Map.MapMarker)
			Push("marker");
			GetVariable();
			Push("Map");
			GetVariable();
			Push("MapMarker");
			GetMember();
			InstanceOf();
			Not();
			If(endLbl);

			// if (marker._parent._parent instanceof Map.LocalMap)
			Push("marker");
			GetVariable();
			Push("_parent");
			GetMember();
			Push("_parent");
			GetMember();
			Push("Map");
			GetVariable();
			Push("LocalMap");
			GetMember();
			InstanceOf();
			Not();
			If(doorLbl);

			// marker.IconClip._alpha = 100;
			Push("marker");
			GetVariable();
			Push("IconClip");
			GetMember();
			Push("_alpha");
			Push(100);
			SetMember();

			// marker._iconName = "DoorMarker";
			Push("marker");
			GetVariable();
			Push("_iconName");
			Push("DoorMarker");
			SetMember();

			// doorLbl:
			L(doorLbl);

			if (a_iconScale != 1.0f) {
				// marker._width *= a_iconScale;
				Push("marker");
				GetVariable();
				Push("_width");
				Push("marker");
				GetVariable();
				Push("_width");
				GetMember();
				Push(a_iconScale);
				Multiply();
				SetMember();

				// marker._height *= a_iconScale;
				Push("marker");
				GetVariable();
				Push("_height");
				Push("marker");
				GetVariable();
				Push("_height");
				GetMember();
				Push(a_iconScale);
				Multiply();
				SetMember();
			}

			// endLbl:
			L(endLbl);
		}
	};
	Action action{ a_iconScale };
	action.Ready();
	auto bufferData = action.GetCode();
	assert(bufferData);

	return TagFactory::MakeDoAction(a_alloc, bufferData);
}
