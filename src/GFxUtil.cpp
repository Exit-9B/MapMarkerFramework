#include "GFxUtil.h"

auto GFxUtil::GetMovieDefImpl(RE::GFxMovieDef* a_movieDef) -> RE::GFxMovieDefImpl*
{
	// SkyrimSE 1.5.97.0: 0x017DD860
	static REL::Relocation<std::uintptr_t> GFxMovieDefImpl_vtbl{ REL::ID(292388), 0x278 };

	if (*reinterpret_cast<std::uintptr_t*>(a_movieDef) == GFxMovieDefImpl_vtbl.get()) {
		return static_cast<RE::GFxMovieDefImpl*>(a_movieDef);
	}
	else {
		logger::critical("GFxMovieDef did not have the expected virtual table address"sv);
		return nullptr;
	}
}
