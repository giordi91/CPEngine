#pragma once
#include "CPEngine/core/window.h"
#include "CPEngine/core/events/event.h"
#include <vector>

namespace cp {
class Application {
public:
  Application();
  virtual ~Application();
  virtual void run();

  void onEvent(core::Event &e);
  //void queueEventForEndOfFrame(core::Event *e);
  //void pushLayer(Layer *layer);
  //void pushOverlay(Layer *layer);

private:
bool onCloseWindow(core::WindowCloseEvent &e);
bool onResizeWindow(core::WindowResizeEvent &e);
  /*
void parseConfigFile();
inline void flipEndOfFrameQueue() {
m_queueEndOfFrameCounter = (m_queueEndOfFrameCounter + 1) % 2;
m_queuedEndOfFrameEventsCurrent =
  &m_queuedEndOfFrameEvents[m_queueEndOfFrameCounter];
};
*/

private:
  core::BaseWindow *m_window = nullptr;
  bool m_run = true;
std::vector<std::vector<core::Event *>> m_queuedEndOfFrameEvents;
std::vector<core::Event *> *m_queuedEndOfFrameEventsCurrent;
uint32_t m_queueEndOfFrameCounter = 0;
  /*
LayerStack m_layerStack;
Layer *imGuiLayer;
Layer *graphicsLayer;
*/
};

// To be implemented by the client
Application *createApplication();
} // namespace cp
