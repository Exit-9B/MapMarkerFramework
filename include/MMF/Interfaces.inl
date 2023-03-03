/**
 * @file Interfaces.inl
 *
 * Copyright (c) Parapets
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "Interfaces.h"

namespace MMF
{
	inline RE::MARKER_TYPE MarkerInfo::GetVanillaMarker() const
	{
		return static_cast<RE::MARKER_TYPE>(proxy_.VanillaMarker);
	}

	inline bool MarkerInfo::HasExternalIcon() const
	{
		return proxy_.SourcePath && proxy_.ExportName && ::strlen(proxy_.SourcePath) &&
			::strlen(proxy_.ExportName);
	}

	inline std::string MarkerInfo::GetSourcePath() const
	{
		return proxy_.SourcePath;
	}

	inline std::string MarkerInfo::GetExportName() const
	{
		return proxy_.ExportName;
	}

	inline std::string MarkerInfo::GetExportNameUndiscovered() const
	{
		return proxy_.ExportNameUndiscovered;
	}

	inline float MarkerInfo::GetIconScale() const
	{
		return proxy_.IconScale;
	}

	inline bool MarkerInfo::IsHiddenFromHUD() const
	{
		return proxy_.HideFromHUD;
	}

	inline std::uint32_t MapMarkerInterface::Version() const
	{
		return GetProxy()->interfaceVersion;
	}

	inline RE::MARKER_TYPE MapMarkerInterface::GetVanillaMarkerType(
		RE::TESObjectREFR* a_refr) const
	{
		const auto markerType = GetProxy()->GetVanillaMarkerType(a_refr);
		return static_cast<RE::MARKER_TYPE>(markerType);
	}

	inline std::string MapMarkerInterface::GetCustomMarkerName(RE::TESObjectREFR* a_refr) const
	{
		return GetProxy()->GetCustomMarkerName(a_refr);
	}

	inline MarkerInfo MapMarkerInterface::GetMapMarkerInfo(RE::TESObjectREFR* a_refr) const
	{
		MarkerInfo info{};
		GetProxy()->GetMapMarkerInfo(a_refr, std::addressof(info.proxy_));
		return info;
	}

	inline MarkerInfo MapMarkerInterface::GetLocalMarkerInfo(RE::BGSLocation* a_loc) const
	{
		MarkerInfo info{};
		GetProxy()->GetLocalMarkerInfo(a_loc, std::addressof(info.proxy_));
		return info;
	}

	inline const detail::MapMarkerInterface* MapMarkerInterface::GetProxy() const
	{
		return reinterpret_cast<const detail::MapMarkerInterface*>(this);
	}

	inline void QueryMapMarkerInterface(
		const SKSE::MessagingInterface::Message* a_msg,
		MapMarkerInterface*& a_intfc)
	{
		if (!a_msg || ::strcmp(a_msg->sender, "MapMarkerFramework") != 0 ||
			a_msg->type != kMapMarkerInterface) {
			return;
		}

		auto result = static_cast<MapMarkerInterface*>(a_msg->data);

		if (result && result->Version() > MapMarkerInterface::kVersion) {
			SKSE::log::warn("interface definition is out of date"sv);
		}

		a_intfc = result;
	}
}
