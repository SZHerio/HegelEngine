#include "engine/core/Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace HegelEngine::core
{
	std::shared_ptr<spdlog::logger> Log::s_coreLogger;
	std::shared_ptr<spdlog::logger> Log::s_clientLogger;

	void Log::init()
	{
		spdlog::set_pattern("%^[%T] [%n] [%l] [%s:%#] %v%$");
		s_coreLogger = spdlog::stdout_color_mt("ENGINE");
		s_coreLogger->set_level(spdlog::level::trace);
		s_coreLogger->flush_on(spdlog::level::warn);

		s_clientLogger = spdlog::stdout_color_mt("APP");
		s_clientLogger->set_level(spdlog::level::trace);
		s_clientLogger->flush_on(spdlog::level::warn);
	}

	std::shared_ptr<spdlog::logger>& Log::coreLogger()
	{
		return s_coreLogger;
	}

	std::shared_ptr<spdlog::logger>& Log::clientLogger()
	{
		return s_clientLogger;
	}
}