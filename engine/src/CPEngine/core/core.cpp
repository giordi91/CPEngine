#include "CPEngine/core/core.h"
#include "logging.h"

namespace cp::core {
StringPool *STRING_POOL = nullptr;
static uint32_t STRING_POOL_SIZE_IN_BYTES = 32*1024*1024;
const char *ENGINE_VERSION = "0.0.1";

void coreStartUp() {

  // lets create the string pool
	STRING_POOL = new StringPool(STRING_POOL_SIZE_IN_BYTES);

  cp::initializeLogging();
  cp::logCoreInfo("Initializing engine v{0}", ENGINE_VERSION);
}

void coreShutDown() { cp::cleanUpLogging(); }
} // namespace cp::core
