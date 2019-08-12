#pragma once

#pragma once
#include "CPEngine/core/events/event.h"

namespace cp{

class Layer;
class LayerStack final{
public:
  LayerStack();
  ~LayerStack();

  void pushLayer(Layer* layer);
  void popLayer(Layer* layer);

  //when the layer is pushed into the stack
  //the stack will be owning it.
  void pushOverlayLayer(Layer* layer);
  //when the layer is popped, the stack is no longer
  //the owner, the popped layer must be freed manually
  void popOverlayLayer(Layer* layer);

  std::vector<Layer*>::iterator begin() { return m_layers.begin(); };
  std::vector<Layer*>::iterator end() { return m_layers.end(); };
private:
  std::vector<Layer*> m_layers;
  std::vector<Layer*>::iterator m_layerInsert;
};
} // namespace SirEngine
