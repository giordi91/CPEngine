#pragma once


namespace cp {

void initializeLogging();
void cleanUpLogging();

void logCoreTrace(const char* message);
void logCoreInfo(const char* message);
void logCoreWarn(const char* message);
void logCoreError(const char* message);
void logCoreFatal(const char* message);

void logTrace(const char* message);
void logInfo(const char* message);
void logWarn(const char* message);
void logError(const char* message);
void logFatal(const char* message);
} // namespace cp
