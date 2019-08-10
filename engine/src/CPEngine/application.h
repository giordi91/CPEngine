#pragma once
//#include "Window.h"
//#include "core.h"
//
//#include "SirEngine/events/applicationEvent.h"
//#include "SirEngine/events/event.h"
//#include "SirEngine/layerStack.h"
#include <vector>
namespace cp {
class Application {
public:
  Application();
  virtual ~Application()= default;
  virtual void run()=0;

  /*
  void onEvent(Event &e);
  void queueEventForEndOfFrame(Event *e);
  void pushLayer(Layer *layer);
  void pushOverlay(Layer *layer);
  */

private:
  /*
void parseConfigFile();
bool onCloseWindow(WindowCloseEvent &e);
bool onResizeWindow(WindowResizeEvent &e);
inline void flipEndOfFrameQueue() {
m_queueEndOfFrameCounter = (m_queueEndOfFrameCounter + 1) % 2;
m_queuedEndOfFrameEventsCurrent =
  &m_queuedEndOfFrameEvents[m_queueEndOfFrameCounter];
};
*/

private:
  /*
std::vector<std::vector<Event *>> m_queuedEndOfFrameEvents;
std::vector<Event *> *m_queuedEndOfFrameEventsCurrent;
uint32_t m_queueEndOfFrameCounter = 0;
Window *m_window = nullptr;
bool m_run = true;
LayerStack m_layerStack;
Layer *imGuiLayer;
Layer *graphicsLayer;
*/
};

// To be implemented by the client
Application *createApplication();
} // namespace cp
