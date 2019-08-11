#pragma once
#include <cstdint>
#include "CPEngine/core/events/applicationEvent.h"

namespace cp::core {
struct WindowProps {
  const char *title = "Editor";
  uint32_t width = 1280;
  uint32_t height = 720;
  bool vsync = false;
};

struct NativeWindow {
  uint64_t data;
};

class Window {
public:
  using EventCallbackFn = std::function<void(Event &)>;

  virtual ~Window() = default;
  virtual void onUpdate() = 0;
  virtual void onResize(uint32_t width, unsigned int height) = 0;

  [[nodiscard]] virtual uint32_t getWidth() const = 0;
  [[nodiscard]] virtual uint32_t getHeight() const = 0;

  virtual void setEventCallback(const EventCallbackFn &callback) = 0;
  // virtual void *getNativeWindow() const = 0;

  // This needs to be implemented per platform
  static Window *create(const WindowProps &props = WindowProps());
};
} // namespace cp
