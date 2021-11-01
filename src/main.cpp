#include "Hooks.h"
#include "MapConfigLoader.h"
#include "ImportManager.h"

class ScaleformLog : public RE::GFxLog
{
public:
	void LogMessageVarg(
		LogMessageType a_messageType,
		const char* a_fmt,
		std::va_list a_argList) override
	{
		LogMessageType messageType =
			static_cast<LogMessageType>(RE::stl::to_underlying(a_messageType) & 0xF);

		spdlog::level::level_enum level;

		switch (messageType) {
		case LogMessageType::kMessageType_Error:
			level = spdlog::level::level_enum::err;
			break;
		case LogMessageType::kMessageType_Warning:
			level = spdlog::level::level_enum::warn;
			break;
		case LogMessageType::kMessageType_Message:
			level = spdlog::level::level_enum::info;
			break;
		default:
			level = spdlog::level::level_enum::trace;
			break;
		}

		int len = _vscprintf(a_fmt, a_argList);
		if (len == -1) {
			return;
		}

		std::size_t size = static_cast<std::size_t>(len) + 1;

		char* str = static_cast<char*>(malloc(size));
		vsprintf_s(str, size, a_fmt, a_argList);

		std::string_view sv{ str, size };

		spdlog::default_logger()->log(level, sv);
	}
};

inline static ScaleformLog scaleformLog{};

extern "C" DLLEXPORT bool SKSEAPI
	SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
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

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	spdlog::default_logger()->flush();

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	logger::info("{} loaded", Version::PROJECT);

	SKSE::Init(a_skse);

	SKSE::AllocTrampoline(28);

	Hooks::Install();

	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener([](SKSE::MessagingInterface::Message* a_msg)
		{
			switch (a_msg->type) {
			case SKSE::MessagingInterface::kDataLoaded:
				auto scaleformManager = RE::BSScaleformManager::GetSingleton();
				scaleformManager->loader->SetState(
					RE::GFxState::StateType::kLog,
					std::addressof(scaleformLog));

				MapConfigLoader::GetSingleton()->LoadAll();

				ImportManager::GetSingleton()->LoadIcons();

				break;
			}
		});

	spdlog::default_logger()->flush();

	return true;
}
