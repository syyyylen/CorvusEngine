#include "InputSystem.h"
#include <Windows.h>

InputSystem* InputSystem::s_inputSystem = nullptr;

InputSystem::InputSystem(): m_old_mouse_pos(0.0f, 0.0f)
{
}

InputSystem::~InputSystem()
{
    s_inputSystem = nullptr;
}

InputSystem* InputSystem::Get()
{
    return s_inputSystem;
}

void InputSystem::Create()
{
    if(s_inputSystem)
        throw std::exception("Input System already created");

    s_inputSystem = new InputSystem();
}

void InputSystem::Release()
{
    if(!s_inputSystem)
        return;
    
    delete s_inputSystem;
}

void InputSystem::Update()
{
    POINT current_mouse_pos = {};
    ::GetCursorPos(&current_mouse_pos);

    if (m_first_time)
    {
        m_old_mouse_pos = InputListener::Vec2(current_mouse_pos.x, current_mouse_pos.y);
        m_first_time = false;
    }

    if (current_mouse_pos.x != m_old_mouse_pos.X || current_mouse_pos.y != m_old_mouse_pos.Y)
    {
        //THERE IS MOUSE MOVE EVENT
        std::unordered_set<InputListener*>::iterator it = m_listenersSet.begin();

        while (it != m_listenersSet.end())
        {
            (*it)->OnMouseMove(InputListener::Vec2(current_mouse_pos.x, current_mouse_pos.y));
            ++it;
        }
    }
    m_old_mouse_pos = InputListener::Vec2(current_mouse_pos.x, current_mouse_pos.y);
    
    if (::GetKeyboardState(m_keys_state))
    {
        for (unsigned int i = 0; i < 256; i++)
        {
            // KEY IS DOWN
            if (m_keys_state[i] & 0x80)
            {
                std::unordered_set<InputListener*>::iterator it = m_listenersSet.begin();

                while (it != m_listenersSet.end())
                {
                    if (i == VK_LBUTTON)
                    {
                        if (m_keys_state[i] != m_old_keys_state[i]) 
                            (*it)->OnLeftMouseDown(InputListener::Vec2(current_mouse_pos.x, current_mouse_pos.y));
                    }
                    else if (i == VK_RBUTTON)
                    {
                        if (m_keys_state[i] != m_old_keys_state[i])
                            (*it)->OnRightMouseDown(InputListener::Vec2(current_mouse_pos.x, current_mouse_pos.y));
                    }
                    else
                        (*it)->OnKeyDown(i);

                    ++it;
                }
            }
            else // KEY IS UP
                {
                if (m_keys_state[i] != m_old_keys_state[i])
                {
                    std::unordered_set<InputListener*>::iterator it = m_listenersSet.begin();

                    while (it != m_listenersSet.end())
                    {
                        if (i == VK_LBUTTON)
                            (*it)->OnLeftMouseUp(InputListener::Vec2(current_mouse_pos.x, current_mouse_pos.y));
                        else if (i == VK_RBUTTON)
                            (*it)->OnRightMouseUp(InputListener::Vec2(current_mouse_pos.x, current_mouse_pos.y));
                        else
                            (*it)->OnKeyUp(i);
                        
                        ++it;
                    }
                }

                }

        }
        // store current keys state to old keys state buffer
        ::memcpy(m_old_keys_state, m_keys_state, sizeof(unsigned char) * 256);
    }
}

void InputSystem::AddListener(InputListener* Listener)
{
    m_listenersSet.insert(Listener);
}

void InputSystem::RemoveListener(InputListener* Listener)
{
    m_listenersSet.erase(Listener);
}

void InputSystem::SetCursorPosition(const InputListener::Vec2& pos)
{
    ::SetCursorPos(pos.X, pos.Y);
}

void InputSystem::ShowCursor(bool show)
{
    ::ShowCursor(show);
}

bool InputSystem::IsCursorVisible()
{
    CURSORINFO ci = {sizeof(CURSORINFO)};

    if (GetCursorInfo(&ci))
    {
        if(ci.flags == 0)
            return false;
        
        return true;
    }

    return false;
}