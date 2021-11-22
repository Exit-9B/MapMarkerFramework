#include "Patches.h"
#include <xbyak/xbyak.h>

bool Patch::WriteDiscoveryMusicPatch(AssignMusicCallback* a_callback)
{
	// SkyrimSE 1.6.318.0: 0x008B12B0+0x2BB
	REL::Relocation<std::uintptr_t> hook{ Offset::HUDNotifications::ProcessMessage, 0x2BB };

	struct Patch : Xbyak::CodeGenerator
	{
		Patch(std::uintptr_t a_hookAddr, std::uintptr_t a_funcAddr)
		{
			Xbyak::Label funcLbl;
			Xbyak::Label retnLbl;

			mov(edx, ptr[r15 + 0x44]);
			lea(rcx, ptr[rbp - 0x19]);
			call(ptr[rip + funcLbl]);
			jmp(ptr[rip + retnLbl]);

			L(funcLbl);
			dq(a_funcAddr);

			L(retnLbl);
			dq(a_hookAddr + 0x1EB); // SkyrimSE 1.6.318.0: 0x008B156B
		}
	};

	std::uintptr_t funcAddr = reinterpret_cast<std::uintptr_t>(a_callback);
	Patch patch{ hook.address(), funcAddr };
	patch.ready();

	if (patch.getSize() > 0x6B) {
		logger::critical("Patch was too large, failed to install"sv);
		return false;
	}

	REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	return true;
}
