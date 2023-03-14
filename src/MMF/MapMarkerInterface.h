#pragma once

#include "MMF/Stubs.h"
#include "main/ImportData.h"

namespace MMF::Impl
{
	namespace MapMarkerInterface
	{
		constexpr std::uint32_t InterfaceVersion = 1;

		void Dispatch();

		detail::MapMarkerInterface* Get();

		std::uint16_t GetVanillaMarkerType(RE::TESObjectREFR* a_refr);

		const char* GetCustomMarkerName(RE::TESObjectREFR* a_refr);

		void GetMapMarkerInfo(RE::TESObjectREFR* a_refr, detail::MARKER_INFO* a_infoOut);

		void GetLocalMarkerInfo(RE::BGSLocation* a_location, detail::MARKER_INFO* a_infoOut);

		void SetIconInfo(const IconInfo* a_info, detail::MARKER_INFO* a_infoOut);
	}
}
