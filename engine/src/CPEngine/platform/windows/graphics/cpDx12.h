#pragma once
#include "CPEngine/graphics/renderingContext.h"

namespace cp::graphics {
namespace dx12 {

graphics::RenderingContext *
createDx12RenderingContext(const RenderingContextCreationSettings &settings);

class Dx12RenderingContext final : public RenderingContext {
public:
  explicit Dx12RenderingContext(
      const RenderingContextCreationSettings &settings);
  ~Dx12RenderingContext() = default;
  Dx12RenderingContext(const Dx12RenderingContext &) = delete;
  Dx12RenderingContext &operator=(const Dx12RenderingContext &) = delete;

  bool initializeGraphics() override;
};

} // namespace dx12
} // namespace cp::graphics