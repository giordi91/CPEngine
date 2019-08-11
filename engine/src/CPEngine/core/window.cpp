#include "CPEngine/core/window.h"
#if CP_WINDOWS_PLATFORM
#include  "CPEngine/platform/windows/core/windowsWindow.h" 
#endif

namespace cp::core {
Window *Window::create(const WindowProps &props)
{
	return new windows::WindowsWindow(props);
}
} // namespace cp::core
