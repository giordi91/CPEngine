#pragma once

#include "CPEngine/core/events/event.h"
#include <sstream>

namespace cp::core {
class KeyboardPressEvent final : public Event {
public:
  explicit KeyboardPressEvent(const uint32_t button) : m_button(button) {}

  EVENT_CLASS_TYPE(KeyPressed)
  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)
  const char* toString() const override {
	  /*
    std::stringstream s;
    s << "KeyboardPressEvent: " << m_button;
    return s.str();
	*/
	  return nullptr;
  }
  uint32_t getKeyCode() const { return m_button; }

private:
  uint32_t m_button;
};

class KeyboardReleaseEvent final : public Event {
public:
  explicit KeyboardReleaseEvent(const uint32_t button) : m_button(button) {}

  EVENT_CLASS_TYPE(KeyReleased)
  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)
  const char* toString() const override {
	  /*
    std::stringstream s;
    s << "KeyboardReleaseEvent: " << m_button;
    return s.str();
	*/
	  return nullptr;
  }

  uint32_t getKeyCode() const { return m_button; }

private:
  uint32_t m_button;
};

class KeyTypeEvent final : public Event {
public:
	explicit KeyTypeEvent(const uint32_t button) : m_button(button) {}

  EVENT_CLASS_TYPE(KeyTyped)
  EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryKeyboard)
  const char* toString() const override {
	  /*
    std::stringstream s;
    s << "Keyboard typed char: " << m_button;
    return s.str();
	*/
	  return nullptr;
  }
  inline uint32_t getKeyCode() const { return m_button; }

private:
  uint32_t m_button;
};

} // namespace cp::core
