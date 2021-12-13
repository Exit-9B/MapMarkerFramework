#pragma once

namespace GFxUtil
{
	using AllocateCallback = std::function<void*(std::size_t)>;

	auto GetMovieDefImpl(RE::GFxMovieDef* a_movieDef) -> RE::GFxMovieDefImpl*;

	auto MakeTagList(
		AllocateCallback a_alloc,
		std::initializer_list<RE::GASExecuteTag*> a_tags) -> RE::GFxTimelineDef::ExecuteTagList;

	auto ExtendTagList(
		AllocateCallback a_alloc,
		RE::GFxTimelineDef::ExecuteTagList& a_tagList,
		std::initializer_list<RE::GASExecuteTag*> a_tags) -> RE::GFxTimelineDef::ExecuteTagList;
}
