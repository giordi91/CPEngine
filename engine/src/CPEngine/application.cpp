#include "CPEngine/application.h"
#include "CPEngine/core/window.h"
#include "core/logging.h"
#include "core/core.h"

//#include "SirEngine/globals.h"
//#include "SirEngine/graphics/graphicsCore.h"
//#include "SirEngine/layer.h"
//#include "SirEngine/log.h"
//#include "fileUtils.h"
//#include "layers/graphics3DLayer.h"
//#include "layers/imguiLayer.h"
//#include <random>

namespace cp {

/*
static const std::string CONFIG_PATH = "engineConfig.json";
static const std::string CONFIG_DATA_SOURCE_KEY = "dataSource";
static const std::string CONFIG_STARTING_SCENE_KEY = "startingScene";
static const std::string DEFAULT_STRING = "";

void Application::parseConfigFile() {
  // try to read the configuration file
  nlohmann::json jobj = getJsonObj(CONFIG_PATH);
  globals::DATA_SOURCE_PATH =
      getValueIfInJson(jobj, CONFIG_DATA_SOURCE_KEY, DEFAULT_STRING);
  assert(!globals::DATA_SOURCE_PATH.empty());
  globals::START_SCENE_PATH=
      getValueIfInJson(jobj, CONFIG_STARTING_SCENE_KEY, DEFAULT_STRING);
  assert(!globals::START_SCENE_PATH.empty());
}

*/
Application::Application() {

  // parseConfigFile();

  m_window = core::Window::create();
  m_window->setEventCallback(
      [this](core::Event &e) -> void { this->onEvent(e); });
  // m_queuedEndOfFrameEvents.resize(2);
  // m_queuedEndOfFrameEvents[0].reserve(10);
  // m_queuedEndOfFrameEvents[1].reserve(10);
  // m_queuedEndOfFrameEventsCurrent = &m_queuedEndOfFrameEvents[0];

  // imGuiLayer = new ImguiLayer();
  // graphicsLayer = new Graphics3DLayer();
  // m_layerStack.pushLayer(graphicsLayer);
  // m_layerStack.pushOverlayLayer(imGuiLayer);
  // globals::APPLICATION = this;
}

Application::~Application() { delete m_window; }
void Application::run() {
  while (m_run) {
    // globals::LAST_FRAME_TIME_NS = globals::GAME_CLOCK.getDelta();
    //++globals::TOTAL_NUMBER_OF_FRAMES;
    m_window->onUpdate();
    //    graphics::newFrame();
    //
    //    for (Layer *l : m_layerStack) {
    //      l->onUpdate();
    //    }
    //    graphics::dispatchFrame();
    //
    //    auto currentQueue = m_queuedEndOfFrameEventsCurrent;
    //    flipEndOfFrameQueue();
    //    for (auto e : (*currentQueue)) {
    //      onEvent(*e);
    //      delete e;
    //    }
    //    currentQueue->clear();

	//at the end of the frame we free memory that has been allocated
	//for frame duration only, this can be for example the data for an
	//envent
	core::STRING_POOL->resetFrameMemory();
  }

  /*
  // lets make sure any graphics operation are done
  graphics::stopGraphics();

  // lets clean up the layers, now is safe to free up resources
  for (Layer *l : m_layerStack) {
    l->clear();
  }

  // shutdown anything graphics related;
  graphics::shutdownGraphics();
  */
}
/*
void Application::queueEventForEndOfFrame(Event *e) {
  m_queuedEndOfFrameEventsCurrent->push_back(e);
}
*/
void Application::onEvent(core::Event &e) {
  // close event dispatch
  // SE_CORE_INFO("{0}", e);
  logCoreInfo("{0}",e.toString());
  core::EventDispatcher dispatcher(e);
  dispatcher.dispatch<core::WindowCloseEvent>(
      [this](core::WindowCloseEvent &e) -> bool {
        return (this->onCloseWindow(e));
      });
  if (e.handled()) {
    return;
  }
  dispatcher.dispatch<core::WindowResizeEvent>(
      [this](core::WindowResizeEvent &e) -> bool {
        return (this->onResizeWindow(e));
      });

  if (e.handled()) {
    return;
  }
  /*
  for (auto it = m_layerStack.end(); it != m_layerStack.begin();) {
    (*--it)->onEvent(e);
    if (e.handled()) {
      break;
    }
  }
  */
}
bool Application::onCloseWindow(core::WindowCloseEvent &) {
  // graphics::shutdown();
  m_run = false;
  return true;
}
bool Application::onResizeWindow(core::WindowResizeEvent &e) {

  // TODO: decide how to handle this kind of information like screen size, I was
  // thinking to have render contexts to hanlde this with a camera etc
  uint32_t w = e.getWidth();
  uint32_t h = e.getHeight();

  m_window->onResize(w, h);
  // graphics::onResize(w, h);

  /*
  // push the resize event to everyone in case is needed
  for (auto it = m_layerStack.end(); it != m_layerStack.begin();) {
    (*--it)->onEvent(e);
    if (e.handled()) {
      break;
    }
  }
  */
  return true;
}

/*
void Application::pushLayer(Layer *layer) { m_layerStack.pushLayer(layer); }
void Application::pushOverlay(Layer *layer) {
  m_layerStack.pushOverlayLayer(layer);
}
*/
} // namespace cp
