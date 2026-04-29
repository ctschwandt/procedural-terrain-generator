#ifndef APP_H
#define APP_H

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "camera.h"
#include "input.h"
#include "scene.h"
#include <string>

class App
{
public:
    App();

    bool init();
    void run();
    void shutdown();

    void on_framebuffer_resize(int width, int height);
    void on_key(int key, int action);
    void on_mouse_button(int button, int action);
    void on_mouse_move(double xpos, double ypos);

private:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

private:
    GLFWwindow* window;
    Camera camera;
    Input input;
    Scene scene;

    int fb_width;
    int fb_height;

    float max_dt;

    bool wireframe_enabled;
    bool show_help_overlay;
    bool show_terrain_panel;
    int active_slider_id;
    bool value_edit_active;
    int edited_value_slider_id;
    std::string value_edit_buffer;
};

#endif
