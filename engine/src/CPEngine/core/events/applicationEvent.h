#pragma once

#include "CPEngine/core/events/event.h"
#include "CPEngine/core/core.h"

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
    char x[64];
    sprintf_s(x, sizeof(x), "%i", m_width);
    char y[64];
    sprintf_s(y, sizeof(y), "%i", m_height);
    const char *positionsString = STRING_POOL->concatenateFrame(x, y, ",");
    const char *finalString =
        STRING_POOL->concatenateFrame("WindowResizeEvent: ", positionsString);
    return finalString;
  }

  inline unsigned int getWidth() const { return m_width; }
  inline unsigned int getHeight() const { return m_height; }

private:
  unsigned int m_width;
  unsigned int m_height;
};

} // namespace SirEngine
