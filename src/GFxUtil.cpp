#include "GFxUtil.h"

auto GFxUtil::GetMovieDefImpl(RE::GFxMovieDef* a_movieDef) -> RE::GFxMovieDefImpl*
{
	static REL::Relocation<std::uintptr_t> GFxMovieDefImpl_vtbl{ Offset::GFxMovieDefImpl::Vtbl };

	if (*reinterpret_cast<std::uintptr_t*>(a_movieDef) == GFxMovieDefImpl_vtbl.get()) {
		return static_cast<RE::GFxMovieDefImpl*>(a_movieDef);
	}
	else {
		logger::critical("GFxMovieDef did not have the expected virtual table address"sv);
		return nullptr;
	}
}

auto GFxUtil::MakeTagList(
	AllocateCallback a_alloc,
	std::initializer_list<RE::GASExecuteTag*> a_tags) -> RE::GFxTimelineDef::ExecuteTagList
{
	std::size_t size = sizeof(RE::GASExecuteTag*) * a_tags.size();
	auto tagArray = static_cast<RE::GASExecuteTag**>(a_alloc(size));

	std::copy(a_tags.begin(), a_tags.end(), tagArray);

	return RE::GFxTimelineDef::ExecuteTagList{
		.data = tagArray,
		.size = static_cast<std::uint32_t>(a_tags.size()),
	};
}

auto GFxUtil::ExtendTagList(
	AllocateCallback a_alloc,
	RE::GFxTimelineDef::ExecuteTagList& a_tagList,
	std::initializer_list<RE::GASExecuteTag*> a_tags) -> RE::GFxTimelineDef::ExecuteTagList
{
	std::size_t size = sizeof(RE::GASExecuteTag*) * (a_tagList.size + a_tags.size());
	auto tagArray = static_cast<RE::GASExecuteTag**>(a_alloc(size));

	std::copy_n(a_tagList.data, a_tagList.size, tagArray);
	std::copy(a_tags.begin(), a_tags.end(), tagArray + a_tagList.size);

	return RE::GFxTimelineDef::ExecuteTagList{
		.data = tagArray,
		.size = a_tagList.size + static_cast<std::uint32_t>(a_tags.size()),
	};
}
