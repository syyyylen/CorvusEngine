#include "CorvusEditor.h"

CorvusEditor::CorvusEditor()
{
    LOG(Debug, "Starting Corvus Editor");
    
    m_window = std::make_shared<Window>(1280, 720, L"Corvus Editor");
}

CorvusEditor::~CorvusEditor()
{
    LOG(Debug, "Destroying Corvus Editor");
    Logger::WriteLogsToFile();
}

void CorvusEditor::Run()
{
    while(m_window->IsRunning())
        m_window->BroadCast();
}
