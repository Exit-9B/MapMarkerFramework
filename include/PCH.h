#pragma once

#include "RE/Skyrim.h"
#include "RE/Ext.h"
#include "SKSE/SKSE.h"
#include <unordered_set>
#include <tsl/ordered_map.h>

#pragma warning(push)
#ifdef NDEBUG
#	include <spdlog/sinks/basic_file_sink.h>
#else
#	include <spdlog/sinks/msvc_sink.h>
#endif
#pragma warning(pop)

using namespace std::literals;

namespace logger = SKSE::log;

#define DLLEXPORT __declspec(dllexport)

#include "Version.h"
