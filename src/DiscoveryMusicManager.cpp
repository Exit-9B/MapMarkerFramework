#include "DiscoveryMusicManager.h"

#include <xbyak/xbyak.h>

auto DiscoveryMusicManager::GetSingleton() -> DiscoveryMusicManager*
{
	static DiscoveryMusicManager singleton{};
	return std::addressof(singleton);
}

void DiscoveryMusicManager::InstallHooks()
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

	if (patch.getSize() > 0x16A) {
		logger::critical("Hook was too large, failed to install"sv);
		return;
	}

	REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	logger::info("Installed hook for discovery music"sv);
}

void DiscoveryMusicManager::SetMusic(RE::MARKER_TYPE a_markerType, std::string_view a_musicType)
{
	if (GetMusic(a_markerType) == a_musicType) {
		return;
	}
	else if (a_musicType == DiscoveryMusic::Generic) {
		_lookup.erase(a_markerType);
		return;
	}

	if (!RE::TESForm::LookupByEditorID<RE::BGSMusicType>(a_musicType)) {
		logger::warn(
			"'{}' did not correspond to a valid Music Type"sv,
			a_musicType);
		// note: allow it to load anyway to help people catch errors
	}

	_lookup[a_markerType] = a_musicType;
}

auto DiscoveryMusicManager::GetMusic(RE::MARKER_TYPE a_markerType) -> std::string_view
{
	auto it = _lookup.find(a_markerType);
	if (it != _lookup.end()) {
		return it->second;
	}
	else {
		return DiscoveryMusic::Generic;
	}
}

auto DiscoveryMusicManager::AssignMusic(
	RE::BSFixedString* a_musicType,
	RE::MARKER_TYPE a_markerType) -> RE::BSFixedString*
{
	*a_musicType = GetSingleton()->GetMusic(a_markerType);
	return a_musicType;
}
