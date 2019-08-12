#pragma once

#include "CPEngine/graphics/layers/layer.h"

namespace cp {
namespace core {
class WindowResizeEvent;
class MouseMoveEvent;
class MouseButtonReleaseEvent;
class MouseButtonPressEvent;

} // namespace core

class Graphics3DLayer final : public Layer {
public:
  Graphics3DLayer() : Layer("GraphicsLayer") {}
  ~Graphics3DLayer() override = default;

  void onAttach() override;
  void onDetach() override;
  void onUpdate() override;
  void onEvent(core::Event &event) override;
  void clear() override;

private:
  // event implementation for the layer
  bool onMouseButtonPressEvent(core::MouseButtonPressEvent &e);
  bool onMouseButtonReleaseEvent(core::MouseButtonReleaseEvent &e);
  bool onMouseMoveEvent(core::MouseMoveEvent &e);
  bool onResizeEvent(core::WindowResizeEvent &e);

  // camera event control
  bool leftDown = false;
  bool rightDown = false;
  bool middleDown = false;
  float previousX = 0;
  float previousY = 0;
};
} // namespace cp
