#pragma once
#include "CPEngine/core/window.h"
#include "CPEngine/core/events/event.h"

#ifndef _AMD64_
#define _AMD64_
#endif
#include <windef.h>

namespace cp::windows {

class WindowsWindow final : public core::BaseWindow {

public:
  explicit WindowsWindow(const core::WindowProps &props);
  virtual ~WindowsWindow() = default;
  void onUpdate() override;
  bool onResize(unsigned int width, unsigned int height) override;

  uint32_t getWidth() const override;
  uint32_t getHeight() const override;

  // window attributes

  void setEventCallback(const EventCallbackFn &callback) override;
  inline EventCallbackFn getEventCallback() const { return m_callback; }

private:
  HINSTANCE m_hinstance;
  HWND m_hwnd;
  core::WindowProps m_data;
  EventCallbackFn m_callback;
};
} // namespace cp::windows
