#include "Hooks.h"
#include "MapConfigLoader.h"
#include "Settings.h"

void InitLogger()
{
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
	}
	else {
		return;
	}

#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return;
	}

	*path /= fmt::format("{}.log"sv, Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT SKSE::PluginVersionData SKSEPlugin_Version =
{
	.dataVersion = SKSE::PluginVersionData::kVersion,

	.pluginVersion = Version::MAJOR,
	.name = PROJECT_NAME,

	.author = "Parapets",
	.supportEmail = "",

	.versionIndependence = 0,
	.compatibleVersions = { SKSE::RUNTIME_1_6_318.packed(), 0 },

	.seVersionRequired = 0,
};

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitLogger();

	logger::info("{} loaded", Version::PROJECT);

	SKSE::Init(a_skse);

	SKSE::AllocTrampoline(14);

	Hooks::Install();

	Settings::GetSingleton()->LoadSettings();

	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener([](SKSE::MessagingInterface::Message* a_msg)
		{
			switch (a_msg->type) {
			case SKSE::MessagingInterface::kDataLoaded:
				MapConfigLoader::GetSingleton()->LoadAll();
				break;
			}
		});

	spdlog::default_logger()->flush();

	return true;
}

