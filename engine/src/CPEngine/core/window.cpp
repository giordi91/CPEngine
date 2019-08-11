#include "CPEngine/core/window.h"
#if CP_WINDOWS_PLATFORM
#include  "CPEngine/platform/windows/core/windowsWindow.h" 
#endif

namespace cp::core {
Window *Window::create(const WindowProps &props)
{

#if CP_WINDOWS_PLATFORM
	//return new windows::WindowsWindow(props);
#endif
    assert(!"No avaialbe window to create for this OS");
}
} // namespace cp::core
