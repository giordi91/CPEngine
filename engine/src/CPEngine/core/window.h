#pragma once
#include "CPEngine/core/events/applicationEvent.h"
#include <cstdint>

namespace cp::core {
struct WindowProps {
  const char *title = "Editor";
  uint32_t width = 1280;
  uint32_t height = 720;
  bool vsync = false;
};

struct NativeWindow {
  uint64_t data;
  uint64_t data2;
};

class BaseWindow {
public:
  using EventCallbackFn = std::function<void(Event &)>;

  virtual ~BaseWindow() = default;
  virtual void onUpdate() = 0;
  virtual bool onResize(uint32_t width, unsigned int height) = 0;

  [[nodiscard]] virtual uint32_t getWidth() const = 0;
  [[nodiscard]] virtual uint32_t getHeight() const = 0;

  virtual void setEventCallback(const EventCallbackFn &callback) = 0;
  const NativeWindow *getNativeWindow() const { return &m_nativeWindow; };

  // This needs to be implemented per platform
  static BaseWindow *create(const WindowProps &props = WindowProps());

protected:
  NativeWindow m_nativeWindow;
};
} // namespace cp::core
