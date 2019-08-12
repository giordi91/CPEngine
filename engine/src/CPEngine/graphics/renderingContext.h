#pragma once
#include <cstdint>

namespace cp {

namespace core {
class BaseWindow;
}

namespace graphics {

enum class GRAPHICS_API {
  VULKAN = 1 << 0,
  DX12 = 1 << 1,
};

struct APIConfig {
  bool vsync = false;
  uint32_t apiVersion = 0;
};

struct RenderingContextCreationSettings {
  GRAPHICS_API graphicsAPI;
  APIConfig apiConfig;
  core::BaseWindow *window;
};

class RenderingContext {

public:
  virtual ~RenderingContext() = default;
  //private copy and assignment
  RenderingContext(const RenderingContext &) = delete;
  RenderingContext &operator=(const RenderingContext &) = delete;



  static RenderingContext *
  create(const RenderingContextCreationSettings &settings);
  static bool isAPISupported(const GRAPHICS_API graphicsAPI);
  virtual bool initializeGraphics() = 0;

protected:
  explicit RenderingContext(const RenderingContextCreationSettings &){};
};

} // namespace graphics
} // namespace cp