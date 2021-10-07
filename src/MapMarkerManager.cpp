#include "MapMarkerManager.h"
#include "FormUtil.h"
#include <xbyak/xbyak.h>
#include <json/json.h>

auto MapMarkerManager::GetSingleton() -> MapMarkerManager*
{
	static MapMarkerManager singleton{};
	return std::addressof(singleton);
}

void MapMarkerManager::InstallHooks()
{
	// SkyrimSE 1.5.97.0: 0x00881383
	REL::Relocation<std::uintptr_t> hook{ REL::ID(50758), 0x3B3 };

	struct Patch : Xbyak::CodeGenerator
	{
		Patch(std::uintptr_t a_hookAddr, std::uintptr_t a_funcAddr)
		{
			Xbyak::Label funcLbl;
			Xbyak::Label retnLbl;

			mov(edx, ptr[rdi + 0x44]);
			lea(rcx, ptr[rbp - 0x29]);
			call(ptr[rip + funcLbl]);
			jmp(ptr[rip + retnLbl]);

			L(funcLbl);
			dq(a_funcAddr);

			L(retnLbl);
			dq(a_hookAddr + 0x16A); // SkyrimSE 1.5.97.0: 0x008814ED
		}
	};

	auto funcAddr = reinterpret_cast<std::uintptr_t>(AssignMusic);
	Patch patch{ hook.address(), funcAddr };
	patch.ready();

	REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	logger::info("Installed hook for discovery music"sv);
}

void MapMarkerManager::LoadAllMapMarkers()
{
	const auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (!dataHandler) {
		return;
	}

	auto mapMarkerDir = std::filesystem::path{ "Data\\MapMarkers" };
	for (auto file : dataHandler->files) {
		if (!file) {
			continue;
		}

		auto fileName = std::filesystem::path{ file->fileName };
		fileName.replace_extension("json"sv);
		auto mapMarkerFile = mapMarkerDir / fileName;

		auto dirEntry = std::filesystem::directory_entry{ mapMarkerFile };
		if (dirEntry.exists()) {
			LoadMapMarkersFromFile(mapMarkerFile);
		}
	}
}

void MapMarkerManager::LoadMapMarkersFromFile(std::filesystem::path a_file)
{
	Json::Value root;
	if (auto fs = std::ifstream{ a_file }) {
		fs >> root;
	}

	Json::Value iconDefinitions = root["iconDefinitions"];
	if (iconDefinitions.isArray()) {
		for (auto& iconDef : iconDefinitions) {
			if (!iconDef.isObject()) {
				continue;
			}

			Json::Value index = iconDef["index"];
			Json::Value discoveryMusic = iconDef["discoveryMusic"];

			if (!index.isUInt() || !discoveryMusic.isString()) {
				continue;
			}

			auto markerType = static_cast<RE::MARKER_TYPE>(index.asUInt());
			_discoveryMusic[markerType] = discoveryMusic.asString();
		}
	}

	Json::Value mapMarkers = root["mapMarkers"];
	if (mapMarkers.isArray()) {
		for (auto& mapMarker : mapMarkers) {
			if (!mapMarker.isObject()) {
				continue;
			}

			Json::Value refID = mapMarker["refID"];
			Json::Value icon = mapMarker["icon"];

			if (!refID.isString() || !icon.isUInt()) {
				continue;
			}

			auto markerRef = skyrim_cast<RE::TESObjectREFR*>(
				FormUtil::GetFormFromIdentifier(refID.asString()));

			if (markerRef) {
				auto markerType = static_cast<RE::MARKER_TYPE>(icon.asUInt());
				UpdateMapMarker(markerRef, markerType);
			}
		}
	}
}

auto MapMarkerManager::GetDiscoveryMusic(RE::MARKER_TYPE a_type) -> const char*
{
	auto it = _discoveryMusic.find(a_type);
	if (it != _discoveryMusic.end()) {
		return it->second.data();
	}
	else {
		return DiscoveryMusicGeneric.data();
	}
}

void MapMarkerManager::UpdateMapMarker(RE::TESObjectREFR* a_markerRef, RE::MARKER_TYPE a_icon)
{
	auto extraMapMarker = a_markerRef->extraList.GetByType<RE::ExtraMapMarker>();
	if (extraMapMarker) {
		extraMapMarker->mapData->type = a_icon;
	}
}

auto MapMarkerManager::AssignMusic(
	RE::BSFixedString* a_musicType,
	RE::MARKER_TYPE a_markerType) -> RE::BSFixedString*
{
	*a_musicType = GetSingleton()->GetDiscoveryMusic(a_markerType);
	return a_musicType;
}
