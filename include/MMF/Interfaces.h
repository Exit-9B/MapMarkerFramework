/**
 * @file Interfaces.h
 *
 * Copyright (c) Parapets
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "Stubs.h"

#include "SKSE/Interfaces.h"

#include <string>

namespace RE
{
	enum class MARKER_TYPE;
}

namespace MMF
{
	struct MarkerInfo
	{
		/**
		 * Get the vanilla marker index if used.
		 *
		 * Undefined behavior if HasExternalIcon() returns true.
		 */
		[[nodiscard]] RE::MARKER_TYPE GetVanillaMarker() const;

		/**
		 * Get whether the marker uses a custom external icon.
		 */
		[[nodiscard]] bool HasExternalIcon() const;

		/**
		 * Get the path to the source SWF for the custom icon.
		 */
		[[nodiscard]] std::string GetSourcePath() const;

		/**
		 * Get the export name of the sprite with the custom icon (discovered).
		 */
		[[nodiscard]] std::string GetExportName() const;

		/**
		 * Get the export name of the sprite with the custom icon (undiscovered).
		 */
		[[nodiscard]] std::string GetExportNameUndiscovered() const;

		/**
		 * Get the scale of the icon.
		 *
		 * Always returns 1.0 for vanilla markers.
		 */
		[[nodiscard]] float GetIconScale() const;

		/**
		 * Get whether the marker is hidden from the HUD compass.
		 */
		[[nodiscard]] bool IsHiddenFromHUD() const;

		detail::MARKER_INFO proxy_;
	};
	static_assert(sizeof(MarkerInfo) == 0x28);

	class MapMarkerInterface
	{
	public:
		enum
		{
			kVersion = 1,
		};

		/**
		 * Get the version of the interface.
		 */
		[[nodiscard]] std::uint32_t Version() const;

		/**
		 * Get the vanilla marker type from the marker's extra data list.
		 *
		 * @param[in] a_refr The map marker reference to query.
		 */
		[[nodiscard]] RE::MARKER_TYPE GetVanillaMarkerType(RE::TESObjectREFR* a_refr) const;

		/**
		 * Get the name used to identify the custom icon if the marker has one.
		 *
		 * @param[in] a_refr The map marker reference to query.
		 */
		[[nodiscard]] std::string GetCustomMarkerName(RE::TESObjectREFR* a_refr) const;

		/**
		 * Get world map marker info for a map marker reference.
		 *
		 * @param[in] a_refr The map marker reference to query.
		 */
		[[nodiscard]] MarkerInfo GetMapMarkerInfo(RE::TESObjectREFR* a_refr) const;

		/**
		 * Get local map marker info for a location.
		 *
		 * @param[in] a_location The location to query.
		 */
		[[nodiscard]] MarkerInfo GetLocalMarkerInfo(RE::BGSLocation* a_location) const;

	protected:
		[[nodiscard]] const detail::MapMarkerInterface* GetProxy() const;
	};

	/**
	 * Try to get the MapMarkerInterface from a message.
	 *
	 * @param[in]  a_msg   The message sent by MapMarkerFramework.
	 * @param[out] a_intfc The variable to store the interface, if present.
	 */
	void QueryMapMarkerInterface(
		const SKSE::MessagingInterface::Message* a_msg,
		MapMarkerInterface*& a_intfc);
}

#include "Interfaces.inl"
