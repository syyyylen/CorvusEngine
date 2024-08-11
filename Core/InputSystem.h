#pragma once

#include "InputListener.h"
#include <unordered_set>

class InputSystem
{
private:
    InputSystem();
    ~InputSystem();

public:
    static InputSystem* Get();
    static void Create();
    static void Release();

    void Update();
    void AddListener(InputListener* Listener);
    void RemoveListener(InputListener* Listener);

    void SetCursorPosition(const InputListener::Vec2& pos);
    void ShowCursor(bool show);
    bool IsCursorVisible();

private:
    std::unordered_set<InputListener*> m_listenersSet;
    unsigned char m_keys_state[256] = {};
    unsigned char m_old_keys_state[256] = {};
    InputListener::Vec2 m_old_mouse_pos;
    bool m_first_time = true;

    static InputSystem* s_inputSystem;
};