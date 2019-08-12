#include "CPEngine/platform/windows/graphics/cpDx12.h"
#include "CPEngine/graphics/renderingContext.h"

namespace cp::graphics::dx12 {

graphics::RenderingContext *
createDx12RenderingContext(const RenderingContextCreationSettings& settings)
{
  return new Dx12RenderingContext(settings);
}

Dx12RenderingContext::Dx12RenderingContext(
    const RenderingContextCreationSettings &settings)
    : RenderingContext(settings) {}

bool Dx12RenderingContext::initializeGraphics() { return true; }
} // namespace cp::graphics::dx12
