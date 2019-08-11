#pragma once
#include "CPEngine/core/window.h"
#include "CPEngine/core/events/event.h"



#include <xcb/xcb.h>
namespace cp::linux{

class LinuxWindow final : public core::BaseWindow {

public:
  explicit LinuxWindow(const core::WindowProps &props);
  virtual ~LinuxWindow() = default;
  void onUpdate() override;
  void onResize(unsigned int width, unsigned int height) override;

  uint32_t getWidth() const override;
  uint32_t getHeight() const override;

  // window attributes
  void setEventCallback(const EventCallbackFn &callback) override;
  inline EventCallbackFn getEventCallback() const { return m_callback; }
  // void *getNativeWindow() const override { return m_hwnd; }
private:
    void handleEvent(const xcb_generic_event_t *event);
    xcb_intern_atom_reply_t* atom_wm_delete_window;

private:
  core::WindowProps m_data;
  EventCallbackFn m_callback;

    xcb_connection_t *connection;
    bool quit = false;
    xcb_screen_t *screen;
    xcb_window_t window;
};
} // namespace cp::windows
