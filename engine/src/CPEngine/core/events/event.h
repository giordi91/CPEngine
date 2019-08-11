#pragma once
#include <functional>
namespace cp::core{

enum class EVENT_TYPE {
  NONE = 0,
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMoved,
  KeyPressed,
  KeyReleased,
  KeyTyped,
  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled,
  DebugLayerChanged,
  DebugRenderChanged,
  RenderGraphChanged,
  ShaderCompile,
  RequestShaderCompile,
  ShaderCompileResult
};

enum EVENT_CATEGORY {
  NONE = 0,
  EventCategoryApplication = 1<<0,
  EventCategoryInput = 1<<1,
  EventCategoryKeyboard = 1<<2,
  EventCategoryMouse = 1<<3,
  EventCategoryMouseButton = 1<<4,
  EventCategoryDebug = 1<<5,
  EventCategoryRendering = 1<<6,
  EventCategoryShaderCompile= 1<<7
};

#define EVENT_CLASS_TYPE(type)                                                 \
  static EVENT_TYPE getStaticType() { return EVENT_TYPE::##type; }               \
  virtual EVENT_TYPE getEventType() const override { return getStaticType(); }  \
  virtual const char *getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category)                                         \
  virtual int getCategoryFlags() const override { return category; }

class Event {
  friend class EventDispatcher;

public:
  virtual ~Event() = default;
  virtual EVENT_TYPE getEventType() const = 0;
  virtual const char *getName() const = 0;
  virtual int getCategoryFlags() const = 0;
  virtual std::string toString() const { return getName(); }

  inline bool isInCategory(EVENT_CATEGORY category) {
    return getCategoryFlags() & category;
  }
  inline bool handled() { return m_handled; };
  inline void setHandled(bool handled) { m_handled = handled; }

protected:
  bool m_handled = false;
};

class EventDispatcher {
  template <typename T> using EventFn = std::function<bool(T &)>;

public:
  EventDispatcher(Event &event) : m_event(event) {}

  // This is the dispatcher, the way the dispatcher works is the following,
  // the dispatcher has been created with an event, which is going to reference
  // to. then when dispatch is called, if the type matches, we are going to call
  // the function on the event the dispatcher was associated to. The function
  // will be
  template <typename T> bool dispatch(EventFn<T> funct) {
    if (m_event.getEventType() == T::getStaticType()) {
      m_event.m_handled = funct(*(T *)&m_event);
      return true;
    }
    return false;
  };

private:
  Event &m_event;
};
inline std::ostream &operator<<(std::ostream &os, const Event &e) {
  return os << e.toString();
}

} // namespace SirEngine
