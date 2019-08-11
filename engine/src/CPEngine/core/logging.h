#pragma once
#include <cstdint>

namespace cp {

// this is a wrapper around the spdlog, unluckily does not allow to use the
// feature with n parameters unless we leak the headers out which I don't want
// to do for the time being, so for now we are going to use overloaded functions
// as many as might be needed
void initializeLogging();
void cleanUpLogging();

void logCoreTrace(const char *message);
void logCoreInfo(const char *message);
void logCoreInfo(const char *message, const char *message2);
void logCoreInfo(const char *message, uint32_t value1, uint32_t value2);
void logCoreWarn(const char *message);
void logCoreError(const char *message);

void logTrace(const char *message);
void logInfo(const char *message);
void logWarn(const char *message);
void logError(const char *message);
} // namespace cp
