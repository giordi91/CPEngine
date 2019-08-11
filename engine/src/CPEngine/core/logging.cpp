
#include "CPEngine/core/logging.h"

#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace cp {

std::shared_ptr<spdlog::logger> s_coreLogger;
std::shared_ptr<spdlog::logger> s_clientLogger;

void initializeLogging() {
  spdlog::set_pattern("%^[%T] %n: %v%$");
  s_coreLogger = spdlog::stdout_color_mt("SirEngine");
  s_coreLogger->set_level(spdlog::level::trace);
  s_clientLogger = spdlog::stdout_color_mt("APP");
  s_clientLogger->set_level(spdlog::level::trace);
}

void cleanUpLogging() {
  s_coreLogger.reset();
  s_clientLogger.reset();
}

void logCoreTrace(const char *message) { s_coreLogger->trace(message); }
void logCoreInfo(const char *message) { s_coreLogger->info(message); }
void logCoreInfo(const char *message, const char* message2) { s_coreLogger->info(message, message2); }
void logCoreInfo(const char *message, uint32_t value1, uint32_t value2) { s_coreLogger->info(message, value1,value2); }
void logCoreWarn(const char *message) { s_coreLogger->warn(message); }
void logCoreError(const char *message) { s_coreLogger->error(message); }
void logTrace(const char *message) { s_clientLogger->trace(message); }
void logInfo(const char *message) { s_clientLogger->info(message); }
void logWarn(const char *message) { s_clientLogger->warn(message); }
void logError(const char *message) { s_clientLogger->error(message); }
void logFatal(const char *message) { s_clientLogger->error(message); }
} // namespace cp
