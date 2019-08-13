#pragma once
#include "CPEngine/core/events/event.h"
#include "CPEngine/core/window.h"
#include "CPEngine/graphics/layers/layer.h"
#include "CPEngine/graphics/layers/layerStack.h"
#include "CPEngine/graphics/renderingContext.h"
#include <vector>

namespace cp {

struct RuntimeApplicationData {
  uint32_t currentFrame = 0;
  uint32_t windowWidth = 0;
  uint32_t windowHeight = 0;
};

class Application {
public:
  Application();
  virtual ~Application();
  // private copy and assignment
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  virtual void run();

  //getters
  const RuntimeApplicationData* getRuntimeDataReadOnly() const {return &m_runtimeData;}
  RuntimeApplicationData* getRuntimeData() {return &m_runtimeData;}

  //layer event stuff
  void onEvent(core::Event &e);
  void queueEventForEndOfFrame(core::Event *e);
  void pushLayer(Layer *layer);
  void pushOverlay(Layer *layer);


private:
  bool onCloseWindow(core::WindowCloseEvent &e);
  bool onResizeWindow(core::WindowResizeEvent &e);
  /*
void parseConfigFile();
*/
  inline void flipEndOfFrameQueue() {
    m_queueEndOfFrameCounter = (m_queueEndOfFrameCounter + 1) % 2;
    m_queuedEndOfFrameEventsCurrent =
        &m_queuedEndOfFrameEvents[m_queueEndOfFrameCounter];
  };

private:
  core::BaseWindow *m_window = nullptr;
  bool m_run = true;
  std::vector<std::vector<core::Event *>> m_queuedEndOfFrameEvents;
  std::vector<core::Event *> *m_queuedEndOfFrameEventsCurrent;
  uint32_t m_queueEndOfFrameCounter = 0;
  graphics::RenderingContext *m_renderingContext;
  LayerStack m_layerStack;
  Layer *graphicsLayer;
  RuntimeApplicationData m_runtimeData;
};

// To be implemented by the client
Application *createApplication();
} // namespace cp
