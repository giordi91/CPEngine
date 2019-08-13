#pragma once
#include "CPEngine/core/memory/stringPool.h"

namespace cp {
namespace core {

extern StringPool* STRING_POOL;
extern const char *ENGINE_VERSION;


// this function initializes the most basic part of the engine, for example
// string pools, logging etc
void coreStartUp();
void coreShutDown();

#ifndef ARRAYSIZE
#define ARRAYSIZE(array) sizeof(array) / sizeof(array[0]);
#endif

} // namespace core
} // namespace cp
