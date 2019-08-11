#pragma once

#include "CPEngine/core/events/event.h"
#include <sstream>

namespace cp::core {
class MouseMoveEvent final : public Event {
public:
  MouseMoveEvent(float posX, float posY) : m_posX(posX), m_posY(posY) {}

  EVENT_CLASS_TYPE(MouseMoved)
  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
  const char *toString() const override {

    char x[64];
    sprintf_s(x, sizeof(x), "%f", m_posX);
    char y[64];
    sprintf_s(y, sizeof(y), "%f", m_posY);
    const char *positionsString = STRING_POOL->concatenateFrame(x, y, ",");
    const char* finalString = STRING_POOL->concatenateFrame("MouseMoveEvent ", positionsString);
	return finalString;

    // s << "MouseMoveEvent: (" << m_posX << "," << m_posY << ")";
    // return s.str();
  }
  inline float getX() const { return m_posX; }
  inline float getY() const { return m_posY; }

private:
  float m_posX;
  float m_posY;
};

class MouseScrollEvent final : public Event {
public:
  MouseScrollEvent(const float offsetX, const float offsetY)
      : m_offsetX(offsetX), m_offsetY(offsetY) {}

  EVENT_CLASS_TYPE(MouseScrolled)
  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
  const char *toString() const override {
    // std::stringstream s;
    // s << "MouseScrollEvent: (" << m_offsetX << "," << m_offsetY << ")";
    // return s.str();
    return nullptr;
  }

  inline float getOffsetX() const { return m_offsetX; }
  inline float getOffsetY() const { return m_offsetY; }

private:
  float m_offsetX;
  float m_offsetY;
};

enum class MOUSE_BUTTONS_EVENT { LEFT = 0, RIGHT, MIDDLE };

class MouseButtonPressEvent final : public Event {
public:
  explicit MouseButtonPressEvent(const MOUSE_BUTTONS_EVENT mouseButton)
      : m_button(mouseButton) {}

  EVENT_CLASS_TYPE(MouseButtonPressed)
  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
  const char *toString() const override {
    /*
std::stringstream s;
s << "MouseButtonPressEvent: ";
switch (m_button) {
case (MOUSE_BUTTONS_EVENT::LEFT): {
s << "left";
break;
}
case (MOUSE_BUTTONS_EVENT::RIGHT): {
s << "right";
break;
}
case (MOUSE_BUTTONS_EVENT::MIDDLE): {
s << "middle";
break;
}
}
return s.str();
  */
    return nullptr;
  }
  inline MOUSE_BUTTONS_EVENT getMouseButton() const { return m_button; };

private:
  MOUSE_BUTTONS_EVENT m_button;
};

class MouseButtonReleaseEvent final : public Event {
public:
  explicit MouseButtonReleaseEvent(const MOUSE_BUTTONS_EVENT mouseButton)
      : m_button(mouseButton) {}

  EVENT_CLASS_TYPE(MouseButtonReleased)
  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
  const char *toString() const override {
    /*
std::stringstream s;
s << "MouseButtonReleaseEvent: ";
switch (m_button) {
case (MOUSE_BUTTONS_EVENT::LEFT): {
s << "left";
break;
}
case (MOUSE_BUTTONS_EVENT::RIGHT): {
s << "right";
break;
}
case (MOUSE_BUTTONS_EVENT::MIDDLE): {
s << "middle";
break;
}
}
return s.str();
  */
    return nullptr;
  }
  inline MOUSE_BUTTONS_EVENT getMouseButton() const { return m_button; };

private:
  MOUSE_BUTTONS_EVENT m_button;
};
} // namespace cp::core
