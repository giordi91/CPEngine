#include <CPEngine/core/entryPoint.h>
#include <CPEngine/application.h>
#include <iostream>

class Editor final : public cp::Application {
public:
  Editor() = default;
  ~Editor() = default;
  void run() override {std::cout<<"hello world"<<std::endl;}

};


cp::Application* cp::createApplication()
{
	return new Editor();
}