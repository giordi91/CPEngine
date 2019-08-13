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
  //here you can put per api flags
  uint32_t genericApiFlags =0;
};

struct RenderingContextCreationSettings {
  GRAPHICS_API graphicsAPI;
  APIConfig apiConfig;
  core::BaseWindow *window;
  uint32_t inFlightFrames = 2;
  uint32_t width;
  uint32_t height;
  bool isHeadless = false;
};

class RenderingContext {

public:
  virtual ~RenderingContext() = default;
  // private copy and assignment
  RenderingContext(const RenderingContext &) = delete;
  RenderingContext &operator=(const RenderingContext &) = delete;

  static RenderingContext *
  create(const RenderingContextCreationSettings &settings, uint32_t width,
         uint32_t height);
  static bool isAPISupported(const GRAPHICS_API graphicsAPI);
  virtual bool initializeGraphics() = 0;
  virtual bool newFrame() = 0;
  virtual bool dispatchFrame() = 0;
  virtual bool resize(uint32_t width, uint32_t height) = 0;

  inline const RenderingContextCreationSettings &getContextSettings() const {
    return m_settings;
  }

protected:
  explicit RenderingContext(const RenderingContextCreationSettings &settings,
                            uint32_t width, uint32_t height)
      : m_settings(settings){};
  RenderingContextCreationSettings m_settings;
};

} // namespace graphics
} // namespace cp