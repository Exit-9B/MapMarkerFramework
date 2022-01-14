#pragma once

namespace Util
{
	auto GetGFxMovieDefImpl(RE::GFxMovieDef* a_movieDef) -> RE::GFxMovieDefImpl*;

	auto MakeTagList(
		RE::GFxMovieDataDef* a_movieData,
		std::initializer_list<RE::GASExecuteTag*> a_tags) -> RE::GFxTimelineDef::ExecuteTagList;

	auto ExtendTagList(
		RE::GFxMovieDataDef* a_movieData,
		RE::GFxTimelineDef::ExecuteTagList& a_tagList,
		std::initializer_list<RE::GASExecuteTag*> a_tags) -> RE::GFxTimelineDef::ExecuteTagList;
}
