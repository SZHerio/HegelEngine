#pragma once

#include <memory>
#include "spdlog/spdlog.h"

namespace HegelEngine::core
{
	class Log
	{
	public:
		static void init();
		static std::shared_ptr<spdlog::logger>& coreLogger();
		static std::shared_ptr<spdlog::logger>& clientLogger();

	private:
		static std::shared_ptr<spdlog::logger> s_coreLogger;
		static std::shared_ptr<spdlog::logger> s_clientLogger;
	};
}

#define HE_CORE_TRACE(...)		SPDLOG_LOGGER_TRACE(::HegelEngine::core::Log::coreLogger(), __VA_ARGS__)
#define HE_CORE_DEBUG(...)		SPDLOG_LOGGER_DEBUG(::HegelEngine::core::Log::coreLogger(), __VA_ARGS__)
#define HE_CORE_INFO(...)		SPDLOG_LOGGER_INFO(::HegelEngine::core::Log::coreLogger(), __VA_ARGS__)
#define HE_CORE_WARN(...)		SPDLOG_LOGGER_WARN(::HegelEngine::core::Log::coreLogger(), __VA_ARGS__)
#define HE_CORE_ERROR(...)		SPDLOG_LOGGER_ERROR(::HegelEngine::core::Log::coreLogger(), __VA_ARGS__)
#define HE_CORE_CRITICAL(...)		SPDLOG_LOGGER_CRITICAL(::HegelEngine::core::Log::coreLogger(), __VA_ARGS__)

#define HE_APP_TRACE(...)		SPDLOG_LOGGER_TRACE(::HegelEngine::core::Log::clientLogger(), __VA_ARGS__)
#define HE_APP_DEBUG(...)		SPDLOG_LOGGER_DEBUG(::HegelEngine::core::Log::clientLogger(), __VA_ARGS__)
#define HE_APP_INFO(...)		SPDLOG_LOGGER_INFO(::HegelEngine::core::Log::clientLogger(), __VA_ARGS__)
#define HE_APP_WARN(...)		SPDLOG_LOGGER_WARN(::HegelEngine::core::Log::clientLogger(), __VA_ARGS__)
#define HE_APP_ERROR(...)		SPDLOG_LOGGER_ERROR(::HegelEngine::core::Log::clientLogger(), __VA_ARGS__)
#define HE_APP_CRITICAL(...)		SPDLOG_LOGGER_CRITICAL(::HegelEngine::core::Log::clientLogger(), __VA_ARGS__)

#define HE_CORE_FATAL(...)		HE_CORE_CRITICAL(__VA_ARGS__)
#define HE_APP_FATAL(...)		HE_APP_CRITICAL(__VA_ARGS__)