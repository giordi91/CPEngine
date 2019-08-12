#include "CPEngine/graphics/renderingContext.h"
#include "CPEngine/core/logging.h"
#include <cassert>
#if CP_WINDOWS_PLATFORM
#include "CPEngine/platform/windows/graphics/cpDx12.h"
#endif

static const char *GRAPHICS_API_TO_NAME[] = {"Vulkan", "DirectX 12"};

namespace cp::graphics {


RenderingContext *createWindowsRenderingContext(
    const RenderingContextCreationSettings &settings) {

  if (!RenderingContext::isAPISupported(settings.graphicsAPI)) {
    logCoreError(
        "Requested api is not supported on system: {0}",
        GRAPHICS_API_TO_NAME[static_cast<uint32_t>(settings.graphicsAPI)]);
    return nullptr;
  }

  //API is supported we can create the context based on the given API
  switch(settings.graphicsAPI)
  {
  case (GRAPHICS_API::DX12):
    {
      return dx12::createDx12RenderingContext(settings);
    }
  case GRAPHICS_API::VULKAN: ;
      assert(!"Vulkan not yet supported ");
  default: ;
      assert(!"Not supported api tried to be created");
      return nullptr;
  }


}

RenderingContext *
RenderingContext::create(const RenderingContextCreationSettings &settings) {
#if CP_WINDOWS_PLATFORM
  return createWindowsRenderingContext(settings);
#endif
}

bool RenderingContext::isAPISupported(const GRAPHICS_API graphicsAPI) {
#if CP_WINDOWS_PLATFORM
  return graphicsAPI == GRAPHICS_API::DX12;
#endif
}
} // namespace cp::graphics
