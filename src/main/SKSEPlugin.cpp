#include "DiscoveryMusicManager.h"
#include "ImportManager.h"
#include "LocalMapManager.h"
#include "MMF/MapMarkerInterface.h"
#include "MapConfigLoader.h"
#include "Settings.h"
#include "VendorManager.h"

namespace
{
	void InitializeLog()
	{
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = logger::log_directory();
		if (!path) {
			util::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("{}.log"sv, Plugin::NAME);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
		const auto level = spdlog::level::trace;
#else
		const auto level = spdlog::level::info;
#endif

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);
	}
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []()
{
	SKSE::PluginVersionData v{};

	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);
	v.AuthorName("Parapets"sv);

	v.UsesAddressLibrary(true);

	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

	SKSE::Init(a_skse);

	SKSE::AllocTrampoline(42);
	DiscoveryMusicManager::InstallHooks();
	ImportManager::InstallHooks();
	LocalMapManager::InstallHooks();

	Settings::GetSingleton()->LoadSettings();

	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(
		[](SKSE::MessagingInterface::Message* a_msg)
		{
			switch (a_msg->type) {
			case SKSE::MessagingInterface::kPostLoad:
				if (SKSE::GetMessagingInterface()->RegisterListener(
						"InfinityUI",
						ImportManager::GetMovieDefFromInfinityUI)) {
					logger::info("Successfully registered for Infinity UI messages!");
				}
				break;
			case SKSE::MessagingInterface::kDataLoaded:
				MapConfigLoader::GetSingleton()->LoadAll();
				VendorManager::GetSingleton()->Load();
				MMF::Impl::MapMarkerInterface::Dispatch();
				break;
			}
		});

	return true;
}
