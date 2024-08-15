#pragma once

class InputListener
{
public:
    struct Vec2
    {
        Vec2();
        Vec2(float _x, float _y) : X(_x), Y(_y) {}
        float X, Y;
    };
    
    InputListener()
    {
    }

    virtual ~InputListener()
    {
    }

    // Keyboard pure virtual callbacks
    virtual void OnKeyDown(int key) = 0;
    virtual void OnKeyUp(int key) = 0;

    // Mouse pure virtual callbacks
    virtual void OnMouseMove(const Vec2& mousePosition) = 0;
    virtual void OnLeftMouseDown(const Vec2& mousePos) = 0;
    virtual void OnRightMouseDown(const Vec2& mousePos) = 0;
    virtual void OnLeftMouseUp(const Vec2& mousePos) = 0;
    virtual void OnRightMouseUp(const Vec2& mousePos) = 0;
};