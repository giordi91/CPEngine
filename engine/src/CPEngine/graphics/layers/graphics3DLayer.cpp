#include "CPEngine/graphics/layers/graphics3DLayer.h"
#include "CPEngine/core/events/event.h"
#include "CPEngine/core/events/mouseEvent.h"
#include "CPEngine/core/events/applicationEvent.h"

namespace cp{
#define SE_BIND_EVENT_FN(fn) std::bind(&fn, this,std::placeholders::_1)

void Graphics3DLayer::onAttach() {
  //globals::MAIN_CAMERA = new Camera3DPivot();
  //// globals::MAIN_CAMERA->setLookAt(0, 125, 0);
  //// globals::MAIN_CAMERA->setPosition(00, 125, 60);

  //globals::MAIN_CAMERA->setLookAt(0, 14, 0);
  //globals::MAIN_CAMERA->setPosition(00, 14, 10);
  //globals::MAIN_CAMERA->updateCamera();

  //dx12::flushCommandQueue(dx12::GLOBAL_COMMAND_QUEUE);
  //auto *currentFc = &dx12::CURRENT_FRAME_RESOURCE->fc;

  //if (!currentFc->isListOpen) {
  //  dx12::resetAllocatorAndList(currentFc);
  //}

  //globals::ASSET_MANAGER->loadScene(globals::START_SCENE_PATH);
  //dx12::executeCommandList(dx12::GLOBAL_COMMAND_QUEUE, currentFc);
  //dx12::flushCommandQueue(dx12::GLOBAL_COMMAND_QUEUE);

}
void Graphics3DLayer::onDetach() {}
void Graphics3DLayer::onUpdate() {
  // setting up camera for the frame
  //globals::CONSTANT_BUFFER_MANAGER->processBufferedData();
  //globals::RENDERING_CONTEXT->setupCameraForFrame();
  //// evaluating rendering graph
  //dx12::RENDERING_GRAPH->compute();

  //// making any clean up for the mesh manager if we have to
  //dx12::MESH_MANAGER->clearUploadRequests();
}
void Graphics3DLayer::onEvent(core::Event &event) {
  core::EventDispatcher dispatcher(event);
  dispatcher.dispatch<core::MouseButtonPressEvent>(
      SE_BIND_EVENT_FN(Graphics3DLayer::onMouseButtonPressEvent));
  dispatcher.dispatch<core::MouseButtonReleaseEvent>(
      SE_BIND_EVENT_FN(Graphics3DLayer::onMouseButtonReleaseEvent));
  dispatcher.dispatch<core::MouseMoveEvent>(
      SE_BIND_EVENT_FN(Graphics3DLayer::onMouseMoveEvent));
  dispatcher.dispatch<core::WindowResizeEvent>(
      SE_BIND_EVENT_FN(Graphics3DLayer::onResizeEvent));
}

void Graphics3DLayer::clear() {}

bool Graphics3DLayer::onMouseButtonPressEvent(core::MouseButtonPressEvent &e) {
  if (e.getMouseButton() == core::MOUSE_BUTTONS_EVENT::LEFT) {
    leftDown = true;
    return true;
  } else if (e.getMouseButton() == core::MOUSE_BUTTONS_EVENT::RIGHT) {
    rightDown = true;
    return true;
  } else if (e.getMouseButton() == core::MOUSE_BUTTONS_EVENT::MIDDLE) {
    middleDown = true;
    return true;
  }
  return false;
}

bool Graphics3DLayer::onMouseButtonReleaseEvent(core::MouseButtonReleaseEvent &e) {
  if (e.getMouseButton() == core::MOUSE_BUTTONS_EVENT::LEFT) {
    leftDown = false;
    return true;
  } else if (e.getMouseButton() == core::MOUSE_BUTTONS_EVENT::RIGHT) {
    rightDown = false;
    return true;
  } else if (e.getMouseButton() == core::MOUSE_BUTTONS_EVENT::MIDDLE) {
    middleDown = false;
    return true;
  }
  return false;
}

bool Graphics3DLayer::onMouseMoveEvent(core::MouseMoveEvent &e) {
  const float deltaX = previousX - e.getX();
  const float deltaY = previousY - e.getY();
  /*
  if (leftDown) {
    globals::MAIN_CAMERA->rotCamera(deltaX, deltaY);
  } else if (middleDown) {
    globals::MAIN_CAMERA->panCamera(deltaX, deltaY);
  } else if (rightDown) {
    globals::MAIN_CAMERA->zoomCamera(deltaX);
  }
  */

  // storing old position
  previousX = e.getX();
  previousY = e.getY();
  return true;
}


bool Graphics3DLayer::onResizeEvent(core::WindowResizeEvent &e) {
  // propagate the resize to every node of the graph
  //dx12::RENDERING_GRAPH->onResizeEvent(e.getWidth(), e.getHeight());
  return true;
}
}  // namespace SirEngine
