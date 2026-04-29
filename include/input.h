#ifndef INPUT_H
#define INPUT_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Input
{
public:
    Input();

    void begin_frame();
    void end_frame();

    void set_key(int key, int action);
    void set_mouse_button(int button, int action);
    void set_mouse_position(double xpos, double ypos);

    bool is_key_down(int key) const;
    bool is_key_pressed(int key) const;
    bool is_key_released(int key) const;

    bool is_mouse_button_down(int button) const;
    bool is_mouse_button_pressed(int button) const;
    bool is_mouse_button_released(int button) const;

    double get_mouse_x() const;
    double get_mouse_y() const;
    double get_mouse_dx() const;
    double get_mouse_dy() const;

    void reset_mouse_delta_tracking();

private:
    static constexpr int key_count = GLFW_KEY_LAST + 1;
    static constexpr int mouse_button_count = GLFW_MOUSE_BUTTON_LAST + 1;

    bool current_keys[key_count];
    bool previous_keys[key_count];

    bool current_mouse_buttons[mouse_button_count];
    bool previous_mouse_buttons[mouse_button_count];

    double mouse_x;
    double mouse_y;
    double mouse_dx;
    double mouse_dy;

    bool first_mouse;
};

#endif
