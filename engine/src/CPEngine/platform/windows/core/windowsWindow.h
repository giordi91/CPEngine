#pragma once
#include "CPEngine/core/window.h"
#include "CPEngine/core/events/event.h"

#ifndef _AMD64_
#define _AMD64_
#endif
#include <windef.h>

// forward declare
// struct HINSTANCE;
// struct HWND;

namespace cp::windows {

class WindowsWindow : public core::Window {

public:
  explicit WindowsWindow(const core::WindowProps &props);
  virtual ~WindowsWindow() = default;
  void onUpdate() override;
  void onResize(unsigned int width, unsigned int height) override;

  uint32_t getWidth() const override;
  uint32_t getHeight() const override;

  // window attributes

  void setEventCallback(const EventCallbackFn &callback) override;
  inline EventCallbackFn getEventCallback() const { return m_callback; }
  // void *getNativeWindow() const override { return m_hwnd; }

private:
  HINSTANCE m_hinstance;
  HWND m_hwnd;
  core::WindowProps m_data;
  EventCallbackFn m_callback;
};
} // namespace cp::windows
