#include "GFxUtil.h"

namespace Util
{
	auto GetGFxMovieDefImpl(RE::GFxMovieDef* a_movieDef) -> RE::GFxMovieDefImpl*
	{
		static REL::Relocation<std::uintptr_t> GFxMovieDefImpl_vtbl{
			Offset::GFxMovieDefImpl::Vtbl
		};

		if (*reinterpret_cast<std::uintptr_t*>(a_movieDef) == GFxMovieDefImpl_vtbl.get()) {
			return static_cast<RE::GFxMovieDefImpl*>(a_movieDef);
		}
		else {
			logger::critical("GFxMovieDef did not have the expected virtual table address"sv);
			return nullptr;
		}
	}

	auto MakeTagList(
		RE::GFxMovieDataDef* a_movieData,
		std::initializer_list<RE::GASExecuteTag*> a_tags) -> RE::GFxTimelineDef::ExecuteTagList
	{
		std::size_t size = sizeof(RE::GASExecuteTag*) * a_tags.size();
		auto tagArray = static_cast<RE::GASExecuteTag**>(
			a_movieData->loadTaskData->allocator.Alloc(size));

		std::copy(a_tags.begin(), a_tags.end(), tagArray);

		return RE::GFxTimelineDef::ExecuteTagList{
			.data = tagArray,
			.size = static_cast<std::uint32_t>(a_tags.size()),
		};
	}

	auto ExtendTagList(
		RE::GFxMovieDataDef* a_movieData,
		RE::GFxTimelineDef::ExecuteTagList& a_tagList,
		std::initializer_list<RE::GASExecuteTag*> a_tags) -> RE::GFxTimelineDef::ExecuteTagList
	{
		std::size_t size = sizeof(RE::GASExecuteTag*) * (a_tagList.size + a_tags.size());
		auto tagArray = static_cast<RE::GASExecuteTag**>(
			a_movieData->loadTaskData->allocator.Alloc(size));

		std::copy_n(a_tagList.data, a_tagList.size, tagArray);
		std::copy(a_tags.begin(), a_tags.end(), tagArray + a_tagList.size);

		return RE::GFxTimelineDef::ExecuteTagList{
			.data = tagArray,
			.size = a_tagList.size + static_cast<std::uint32_t>(a_tags.size()),
		};
	}
}
