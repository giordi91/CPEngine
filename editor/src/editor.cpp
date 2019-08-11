#include <CPEngine/core/entryPoint.h>
#include <CPEngine/application.h>

class Editor final : public cp::Application {
public:
  Editor() = default;
  ~Editor() = default;
};


cp::Application* cp::createApplication()
{
	return new Editor();
}