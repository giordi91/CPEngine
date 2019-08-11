#pragma once

#include "CPEngine/core/events/event.h"

namespace cp::core{

class WindowCloseEvent : public Event {
public:
  WindowCloseEvent() = default;

  EVENT_CLASS_TYPE(WindowClose)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowResizeEvent : public Event {
public:
  WindowResizeEvent(unsigned int width, unsigned int height)
      : m_width(width), m_height(height) {}

  EVENT_CLASS_TYPE(WindowResize)
  EVENT_CLASS_CATEGORY(EventCategoryApplication)
  const char* toString() const override {
	  /*
    std::stringstream s;
    s << "WindowResizeEvent: " << m_width << "x" << m_height;
    return s.str();
	*/
	  return nullptr;
  }

  inline unsigned int getWidth() const { return m_width; }
  inline unsigned int getHeight() const { return m_height; }

private:
  unsigned int m_width;
  unsigned int m_height;
};

} // namespace SirEngine
