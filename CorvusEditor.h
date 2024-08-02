#pragma once
#include "Core.h"
#include "Window.h"

class CorvusEditor
{
public:
    CorvusEditor();
    ~CorvusEditor();

    void Run();

private:
    std::shared_ptr<Window> m_window;
};
