#pragma once

#include "CPEngine/core/events/event.h"

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
    const char *finalString =
        STRING_POOL->concatenateFrame("MouseMoveEvent ", positionsString);
    return finalString;
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
    char x[64];
    sprintf_s(x, sizeof(x), "%f", m_offsetX);
    char y[64];
    sprintf_s(y, sizeof(y), "%f", m_offsetY);
    const char *positionsString = STRING_POOL->concatenateFrame(x, y, ",");
    const char *finalString =
        STRING_POOL->concatenateFrame("MouseScrolled ", positionsString);
    return finalString;
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

    const char *direction="";
    switch (m_button) {
    case (MOUSE_BUTTONS_EVENT::LEFT): {
      direction = "left";
      break;
    }
    case (MOUSE_BUTTONS_EVENT::RIGHT): {
      direction = "right";
      break;
    }
    case (MOUSE_BUTTONS_EVENT::MIDDLE): {
      direction = "middle";
      break;
    }
    }

    const char *finalString =
        STRING_POOL->concatenateFrame("MouseButtonPressEvent: ", direction);
    return finalString;
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
    const char *button="";
    switch (m_button) {
    case (MOUSE_BUTTONS_EVENT::LEFT): {
      button = "left";
      break;
    }
    case (MOUSE_BUTTONS_EVENT::RIGHT): {
      button = "right";
      break;
    }
    case (MOUSE_BUTTONS_EVENT::MIDDLE): {
      button = "middle";
      break;
    }
    }

    const char *finalString =
        STRING_POOL->concatenateFrame("MouseButtonReleaseEvent: ", button);
    return finalString;
  }
  inline MOUSE_BUTTONS_EVENT getMouseButton() const { return m_button; };

private:
  MOUSE_BUTTONS_EVENT m_button;
};
} // namespace cp::core
