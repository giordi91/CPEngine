#include "CPEngine/core/window.h"
#if CP_WINDOWS_PLATFORM
#include  "CPEngine/platform/windows/core/windowsWindow.h" 
#endif
#if CP_LINUX_PLATFORM
#include  "CPEngine/platform/linux/core/linuxWindow.h"
#endif

namespace cp::core {
BaseWindow *BaseWindow::create(const WindowProps &props)
{

#if CP_WINDOWS_PLATFORM
	//return new windows::WindowsWindow(props);
#endif

#if CP_LINUX_PLATFORM
    return new linux::LinuxWindow(props);
#endif

    assert(!"No avaialbe window to create for this OS");
}
} // namespace cp::core
