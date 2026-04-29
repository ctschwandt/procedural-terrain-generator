#include "input.h"

Input::Input()
    : mouse_x(0.0)
    , mouse_y(0.0)
    , mouse_dx(0.0)
    , mouse_dy(0.0)
    , first_mouse(true)
{
    for (int i = 0; i < key_count; ++i)
    {
        current_keys[i] = false;
        previous_keys[i] = false;
    }

    for (int i = 0; i < mouse_button_count; ++i)
    {
        current_mouse_buttons[i] = false;
        previous_mouse_buttons[i] = false;
    }
}

void Input::begin_frame()
{
    mouse_dx = 0.0;
    mouse_dy = 0.0;
}

void Input::end_frame()
{
    for (int i = 0; i < key_count; ++i)
    {
        previous_keys[i] = current_keys[i];
    }

    for (int i = 0; i < mouse_button_count; ++i)
    {
        previous_mouse_buttons[i] = current_mouse_buttons[i];
    }
}

void Input::set_key(int key, int action)
{
    if (key < 0 || key >= key_count)
    {
        return;
    }

    if (action == GLFW_PRESS)
    {
        current_keys[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        current_keys[key] = false;
    }
}

void Input::set_mouse_button(int button, int action)
{
    if (button < 0 || button >= mouse_button_count)
    {
        return;
    }

    if (action == GLFW_PRESS)
    {
        current_mouse_buttons[button] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        current_mouse_buttons[button] = false;
    }
}

void Input::set_mouse_position(double xpos, double ypos)
{
    if (first_mouse)
    {
        mouse_x = xpos;
        mouse_y = ypos;
        first_mouse = false;
        return;
    }

    mouse_dx += xpos - mouse_x;
    mouse_dy += ypos - mouse_y;
    mouse_x = xpos;
    mouse_y = ypos;
}

bool Input::is_key_down(int key) const
{
    if (key < 0 || key >= key_count)
    {
        return false;
    }
    return current_keys[key];
}

bool Input::is_key_pressed(int key) const
{
    if (key < 0 || key >= key_count)
    {
        return false;
    }
    return current_keys[key] && !previous_keys[key];
}

bool Input::is_key_released(int key) const
{
    if (key < 0 || key >= key_count)
    {
        return false;
    }
    return !current_keys[key] && previous_keys[key];
}

bool Input::is_mouse_button_down(int button) const
{
    if (button < 0 || button >= mouse_button_count)
    {
        return false;
    }
    return current_mouse_buttons[button];
}

bool Input::is_mouse_button_pressed(int button) const
{
    if (button < 0 || button >= mouse_button_count)
    {
        return false;
    }
    return current_mouse_buttons[button] && !previous_mouse_buttons[button];
}

bool Input::is_mouse_button_released(int button) const
{
    if (button < 0 || button >= mouse_button_count)
    {
        return false;
    }
    return !current_mouse_buttons[button] && previous_mouse_buttons[button];
}

double Input::get_mouse_x() const
{
    return mouse_x;
}

double Input::get_mouse_y() const
{
    return mouse_y;
}

double Input::get_mouse_dx() const
{
    return mouse_dx;
}

double Input::get_mouse_dy() const
{
    return mouse_dy;
}

void Input::reset_mouse_delta_tracking()
{
    mouse_dx = 0.0;
    mouse_dy = 0.0;
    first_mouse = true;
}
