#pragma once
#include "CPEngine/core/events/event.h"

namespace cp{

class Layer {

public:

  Layer() : m_debugName("Layer") {}
  Layer(const char *debugName) : m_debugName(debugName) {}
  virtual ~Layer() = default;

  virtual void onAttach() = 0;
  virtual void onDetach() = 0;
  virtual void onUpdate() = 0;
  virtual void onEvent(core::Event &event) = 0;
  virtual void clear() = 0;


  inline const char* getName() const { return m_debugName; }

protected:
  const char* m_debugName;
};

} // namespace SirEngine
