#pragma once
#include <cstdint>

//#include "SirEngine/events/event.h"

namespace cp {
namespace core {
struct WindowProps {
  const char *title = "Editor";
  unsigned int width = 1280;
  unsigned int height = 720;
  bool vsync = false;
};

struct NativeWindow {
  uint64_t data;
};

class Window {
public:
  // using EventCallbackFn = std::function<void(Event &)>;

  virtual ~Window() = default;
  virtual void onUpdate() = 0;
  virtual void onResize(unsigned int width, unsigned int height) = 0;

  virtual unsigned int getWidth() const = 0;
  virtual unsigned int getHeight() const = 0;

  // window attributes
  // virtual void setEventCallback(const EventCallbackFn &callback) = 0;
  // virtual void *getNativeWindow() const = 0;

  // This needs to be implemented per platform
  static Window *create(const WindowProps &props = WindowProps());
};
} // namespace core
} // namespace cp
