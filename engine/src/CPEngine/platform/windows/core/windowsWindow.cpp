
#if CP_WINDOWS_PLATFORM
#include "CPEngine/platform/windows/core/windowsWindow.h"

#include "CPEngine/core/logging.h"

#include "CPEngine/core/core.h"
#include "CPEngine/core/events/keyboardEvent.h"
#include "CPEngine/core/events/mouseEvent.h"
#include <Windows.h>
#include <windowsx.h>

namespace cp::windows {

// specific windows implementation, most notably window proc
// and message pump handle
static WindowsWindow *windowsApplicationHandle = nullptr;
LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam,
                         LPARAM lparam) {

#define ASSERT_CALLBACK_AND_DISPATCH(e)                                        \
  auto callback = windowsApplicationHandle->getEventCallback();                \
  assert(callback != nullptr);                                                 \
  callback(e);

  switch (umessage) {

  case WM_QUIT: {
    core::WindowCloseEvent closeEvent;
    ASSERT_CALLBACK_AND_DISPATCH(closeEvent);
    return 0;
  }
  case WM_CLOSE: {
    core::WindowCloseEvent closeEvent;
    ASSERT_CALLBACK_AND_DISPATCH(closeEvent);
    return 0;
  }
  case WM_CHAR: {
    // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
    if (wparam > 0 && wparam < 0x10000) {
      // basic shortcut handling
      core::KeyTypeEvent e{static_cast<unsigned int>(wparam)};
      ASSERT_CALLBACK_AND_DISPATCH(e);
    }
    return 0;
  }

  case WM_SIZE: {
    // the reason for this check is because the window call a resize immediately
    // before the user has time to set the callback to the window if
    auto callback = windowsApplicationHandle->getEventCallback();
    if (callback != nullptr) {
      auto w = uint32_t(LOWORD(lparam));
      auto h = uint32_t(HIWORD(lparam));
      core::WindowResizeEvent resizeEvent{w, h};
      callback(resizeEvent);
    }
    return 0;
  }
    // Check if a key has been pressed on the keyboard.
  case WM_KEYDOWN: {
    // repeated key message not supported as differentiator for now,
    // if I wanted to do that seems like bit 30 of lparam is the one
    // giving you if the first press or a repeat, not sure how to get the
    //"lag" in before sending repeats.
    auto code = static_cast<unsigned int>(wparam);
    core::KeyboardPressEvent e{code};
    ASSERT_CALLBACK_AND_DISPATCH(e);

    return 0;
  }

    // Check if a key has been released on the keyboard.
  case WM_KEYUP: {
    core::KeyboardReleaseEvent e{static_cast<unsigned int>(wparam)};

#ifdef QUIT_ESCAPE
    // here we hard-coded this behavior where if the VK_ESCAPE button
    // is pressed I want the message to be sent out as close window,
    // this is a personal preference
    if (wparam == VK_ESCAPE) {
      core::WindowCloseEvent closeEvent;
      ASSERT_CALLBACK_AND_DISPATCH(closeEvent);
      return 0;
    }
#endif

    ASSERT_CALLBACK_AND_DISPATCH(e);
    return 0;
  }
  case WM_LBUTTONDOWN: {
    core::MouseButtonPressEvent e{core::MOUSE_BUTTONS_EVENT::LEFT};
    ASSERT_CALLBACK_AND_DISPATCH(e);
    return 0;
  }
  case WM_RBUTTONDOWN: {
    core::MouseButtonPressEvent e{core::MOUSE_BUTTONS_EVENT::RIGHT};
    ASSERT_CALLBACK_AND_DISPATCH(e);
    return 0;
  }
  case WM_MBUTTONDOWN: {
    core::MouseButtonPressEvent e{core::MOUSE_BUTTONS_EVENT::MIDDLE};
    ASSERT_CALLBACK_AND_DISPATCH(e);
    return 0;
  }
  case WM_MOUSEWHEEL: {
    auto movementY = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wparam));
    // side tilt of the scroll currently not supported, always 0.0f
    core::MouseScrollEvent e{0.0f, movementY};
    ASSERT_CALLBACK_AND_DISPATCH(e);
    return 0;
  }
  case WM_LBUTTONUP: {
    core::MouseButtonReleaseEvent e{core::MOUSE_BUTTONS_EVENT::LEFT};
    ASSERT_CALLBACK_AND_DISPATCH(e);
    return 0;
  }
  case WM_RBUTTONUP: {
    core::MouseButtonReleaseEvent e{core::MOUSE_BUTTONS_EVENT::RIGHT};
    ASSERT_CALLBACK_AND_DISPATCH(e);
    return 0;
  }
  case WM_MBUTTONUP: {
    core::MouseButtonReleaseEvent e{core::MOUSE_BUTTONS_EVENT::MIDDLE};
    ASSERT_CALLBACK_AND_DISPATCH(e);
    return 0;
  }
  case WM_MOUSEMOVE: {
    float posX = static_cast<float>(GET_X_LPARAM(lparam));
    float posY = static_cast<float>(GET_Y_LPARAM(lparam));
    core::MouseMoveEvent e{posX, posY};
    ASSERT_CALLBACK_AND_DISPATCH(e);
    return 0;
  }

  // Any other messages send to the default message handler as our application
  // won't make use of them.
  default: {
    return DefWindowProc(hwnd, umessage, wparam, lparam);
  }
  }
}

//// This needs to be implemented per platform
// core::BaseWindow * core::BaseWindow::create(const core::WindowProps &props) {
//  return new WindowsWindow(props);
//};

WindowsWindow::WindowsWindow(const core::WindowProps &props) {

  m_data = props;
  logCoreInfo("Creating WindowsWindow with dimensions: {0}x{1}", props.width,
              props.height);

  WNDCLASSEX wc;

  // Get an external pointer to this object.
  windowsApplicationHandle = this;

  // Get the instance of this application.
  m_hinstance = GetModuleHandle(NULL);

  // Give the application a name.

  // Setup the windows class with default settings.
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = m_hinstance;
  wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
  wc.hIconSm = wc.hIcon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName = NULL;
  const wchar_t *titleW = core::STRING_POOL->convertWide(props.title);
  wc.lpszClassName = titleW;
  wc.cbSize = sizeof(WNDCLASSEX);

  // Register the window class.
  RegisterClassEx(&wc);

  m_data.width = props.width;
  m_data.height = props.height;
  m_data.title = props.title;

  constexpr DWORD style =
      WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  RECT wr{0, 0, (LONG)m_data.width, (LONG)m_data.height};
  // needed to create the window of the right size, or wont match the gui
  AdjustWindowRectEx(&wr, style, false, NULL);
  m_hwnd =
      CreateWindowEx(0, titleW, titleW, style, 0, 0, wr.right - wr.left,
                     wr.bottom - wr.top, NULL, NULL, GetModuleHandle(NULL), 0);

  // Bring the window up on the screen and set it as main focus.
  ShowWindow(m_hwnd, SW_SHOWDEFAULT);
  UpdateWindow(m_hwnd);
  SetForegroundWindow(m_hwnd);
  SetFocus(m_hwnd);

  // Hide the mouse cursor.
  ShowCursor(true);
  core::STRING_POOL->free(titleW);

  // initialize dx12
  // bool result = graphics::initializeGraphics(this, m_data.width,
  // m_data.height); if (!result) {
  //  SE_CORE_ERROR("FATAL: could not initialize graphics");
  //}
}

void WindowsWindow::onUpdate() {

  MSG msg;
  // initialize the message structure.
  ZeroMemory(&msg, sizeof(MSG));

  // Loop until there is a quit message from the window or the user.
  // Handle the windows messages.
  if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void WindowsWindow::onResize(unsigned int width, unsigned int height) {
  m_data.width = width;
  m_data.height = height;
}

unsigned int WindowsWindow::getWidth() const { return m_data.width; }
unsigned int WindowsWindow::getHeight() const { return m_data.height; }
void WindowsWindow::setEventCallback(const EventCallbackFn &callback) {
  m_callback = callback;
}

} // namespace cp::windows
#endif