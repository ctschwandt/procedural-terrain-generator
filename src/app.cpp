#include "app.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <glad/gl.h>

#include "math.h"

namespace
{
    static const char * app_title = "OpenGL Terrain Generation";

    static constexpr int default_window_width = 1100;
    static constexpr int default_window_height = 800;
    static constexpr float max_frame_dt = 0.05f;
    static constexpr std::size_t shader_log_buffer_size = 4096;

    static constexpr int glyph_rows = 7;
    static constexpr int glyph_columns = 5;
    static constexpr float glyph_character_advance = 6.0f;
    static constexpr float glyph_line_height = 8.0f;

    static constexpr float help_overlay_x = 14.0f;
    static constexpr float help_overlay_y = 14.0f;
    static constexpr float help_overlay_width = 250.0f;
    static constexpr float help_overlay_height = 222.0f;
    static constexpr float help_overlay_outline_thickness = 2.0f;
    static constexpr float help_overlay_text_x_offset = 12.0f;
    static constexpr float help_overlay_text_y_start = 12.0f;
    static constexpr float help_overlay_text_y_step = 22.0f;
    static constexpr float help_overlay_text_scale = 1.5f;

    static constexpr float terrain_panel_x = 20.0f;
    static constexpr float terrain_panel_y = 180.0f;
    static constexpr float terrain_panel_width = 520.0f;
    static constexpr float terrain_panel_top_padding = 68.0f;
    static constexpr float terrain_panel_header_height = 0.0f;
    static constexpr float terrain_panel_outline_thickness = 2.0f;
    static constexpr float terrain_panel_title_x_offset = 12.0f;
    static constexpr float terrain_panel_title_y_offset = 10.0f;
    static constexpr float terrain_panel_title_scale = 2.0f;
    static constexpr float terrain_panel_mode_y_offset = 36.0f;
    static constexpr float terrain_panel_mode_scale = 1.5f;
    static constexpr float terrain_slider_row_step = 40.0f;

    static constexpr float slider_row_x_offset = 12.0f;
    static constexpr float slider_row_width_inset = 24.0f;
    static constexpr float slider_label_scale = 1.5f;
    static constexpr float slider_label_x_offset = 8.0f;
    static constexpr float slider_label_y_offset = 8.0f;
    static constexpr float slider_value_box_width = 82.0f;
    static constexpr float slider_value_box_height = 24.0f;
    static constexpr float slider_right_padding = 12.0f;
    static constexpr float slider_gap_after_label = 14.0f;
    static constexpr float slider_gap_after_value_box = 12.0f;
    static constexpr float slider_row_height = 34.0f;
    static constexpr float slider_row_fill_active_alpha = 0.56f;
    static constexpr float slider_row_fill_inactive_alpha = 0.25f;
    static constexpr float slider_track_y_offset = 16.0f;
    static constexpr float slider_track_height = 8.0f;
    static constexpr float slider_knob_width = 10.0f;
    static constexpr float slider_knob_height = 18.0f;
    static constexpr float slider_knob_y_offset = 5.0f;
    static constexpr float slider_track_hit_y_offset = 8.0f;
    static constexpr float slider_track_hit_extra_height = 16.0f;
    static constexpr float slider_value_box_y_offset = 5.0f;
    static constexpr float slider_value_text_x_offset = 6.0f;
    static constexpr float slider_value_text_y_offset = 8.0f;
    static constexpr float slider_outline_thickness = 1.0f;

    static constexpr float terrain_panel_button_gap = 10.0f;
    static constexpr float terrain_panel_bottom_padding = 10.0f;
    static constexpr float terrain_button_height = 32.0f;
    static constexpr float terrain_button_reset_x_offset = 12.0f;
    static constexpr float terrain_button_reset_width = 208.0f;

    struct TerrainSliderRange
    {
        float min_value;
        float max_value;
    };

    static constexpr TerrainSliderRange sinusoidal_amplitude_range = { 1.0f, 100.0f };
    static constexpr TerrainSliderRange sinusoidal_x_frequency_range = { 0.01f, 1.0f };
    static constexpr TerrainSliderRange sinusoidal_z_frequency_range = { 0.01f, 1.0f };

    static constexpr TerrainSliderRange fractal_height_scale_range = { 1.0f, 200.0f };
    static constexpr TerrainSliderRange fractal_base_frequency_range = { 0.001f, 0.200f };
    static constexpr TerrainSliderRange fractal_octaves_range = { 1.0f, 12.0f };
    static constexpr TerrainSliderRange fractal_persistence_range = { 0.10f, 0.99f };
    static constexpr TerrainSliderRange fractal_lacunarity_range = { 1.10f, 4.0f };
    static constexpr TerrainSliderRange fractal_redistribution_range = { 0.50f, 8.0f };
    static constexpr TerrainSliderRange fractal_seed_range = { 0.0f, 1000000.0f };

    static constexpr TerrainSliderRange diamond_square_initial_amplitude_range = { 1.0f, 200.0f };
    static constexpr TerrainSliderRange diamond_square_roughness_range = { 0.10f, 0.95f };
    static constexpr TerrainSliderRange diamond_square_seed_range = { 0.0f, 1000000.0f };
    static constexpr TerrainSliderRange diamond_square_exponent_range = { 5.0f, 9.0f };

    struct UiVertex
    {
        float x;
        float y;
        float r;
        float g;
        float b;
        float a;
    };

    struct UiColor
    {
        float r;
        float g;
        float b;
        float a;
    };

    static constexpr UiColor ui_color_help_overlay_bg = { 0.03f, 0.03f, 0.03f, 0.74f };
    static constexpr UiColor ui_color_help_overlay_outline = { 0.30f, 0.45f, 0.65f, 0.95f };
    static constexpr UiColor ui_color_help_overlay_text = { 0.86f, 0.86f, 0.86f, 1.0f };
    static constexpr UiColor ui_color_panel_bg = { 0.02f, 0.02f, 0.05f, 0.84f };
    static constexpr UiColor ui_color_panel_outline = { 0.30f, 0.50f, 0.76f, 0.95f };
    static constexpr UiColor ui_color_panel_title = { 0.95f, 0.95f, 0.90f, 1.0f };
    static constexpr UiColor ui_color_panel_text = { 0.88f, 0.88f, 0.88f, 1.0f };
    static constexpr UiColor ui_color_button_default = { 0.15f, 0.21f, 0.31f, 0.95f };
    static constexpr UiColor ui_color_button_hover = { 0.20f, 0.29f, 0.42f, 0.98f };
    static constexpr UiColor ui_color_button_pressed = { 0.10f, 0.17f, 0.26f, 0.98f };
    static constexpr UiColor ui_color_button_outline = { 0.45f, 0.65f, 0.90f, 1.0f };
    static constexpr UiColor ui_color_button_text = { 0.95f, 0.95f, 0.95f, 1.0f };
    static constexpr UiColor ui_color_slider_row_active = { 0.18f, 0.26f, 0.38f, slider_row_fill_active_alpha };
    static constexpr UiColor ui_color_slider_row_inactive = { 0.10f, 0.10f, 0.13f, slider_row_fill_inactive_alpha };
    static constexpr UiColor ui_color_slider_label = { 0.92f, 0.92f, 0.92f, 1.0f };
    static constexpr UiColor ui_color_value_box_default = { 0.09f, 0.12f, 0.16f, 0.95f };
    static constexpr UiColor ui_color_value_box_hover = { 0.14f, 0.18f, 0.24f, 0.98f };
    static constexpr UiColor ui_color_value_box_editing = { 0.16f, 0.24f, 0.35f, 0.98f };
    static constexpr UiColor ui_color_value_text = { 0.80f, 0.90f, 0.99f, 1.0f };
    static constexpr UiColor ui_color_slider_track_bg = { 0.13f, 0.14f, 0.18f, 0.95f };
    static constexpr UiColor ui_color_slider_track_fill = { 0.42f, 0.68f, 0.95f, 0.95f };
    static constexpr UiColor ui_color_knob_default = { 0.90f, 0.94f, 0.99f, 0.95f };
    static constexpr UiColor ui_color_knob_hover = { 0.98f, 0.99f, 1.0f, 1.0f };
    static constexpr UiColor ui_color_knob_active = { 1.0f, 1.0f, 1.0f, 1.0f };
    static constexpr UiColor ui_color_knob_outline = { 0.16f, 0.20f, 0.28f, 1.0f };

    class UiRenderer
    {
    public:
        bool init()
        {
            const char * vs_src = R"(#version 420 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;
out vec4 v_color;
void main()
{
    v_color = a_color;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}
)";

            const char * fs_src = R"(#version 420 core
in vec4 v_color;
out vec4 frag_color;
void main()
{
    frag_color = v_color;
}
)";

            GLuint vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, &vs_src, nullptr);
            glCompileShader(vs);
            if (!check_shader_compile(vs))
            {
                glDeleteShader(vs);
                return false;
            }

            GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, &fs_src, nullptr);
            glCompileShader(fs);
            if (!check_shader_compile(fs))
            {
                glDeleteShader(vs);
                glDeleteShader(fs);
                return false;
            }

            program = glCreateProgram();
            glAttachShader(program, vs);
            glAttachShader(program, fs);
            glLinkProgram(program);
            glDeleteShader(vs);
            glDeleteShader(fs);

            GLint linked = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &linked);
            if (!linked)
            {
                char info[shader_log_buffer_size];
                glGetProgramInfoLog(program, GLsizei(sizeof(info)), nullptr, info);
                std::cerr << "UI program link error:\n" << info << "\n";
                return false;
            }

            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);

            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UiVertex), reinterpret_cast<void*>(0));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(UiVertex), reinterpret_cast<void*>(2 * sizeof(float)));
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            return true;
        }

        void shutdown()
        {
            if (vbo != 0)
            {
                glDeleteBuffers(1, &vbo);
                vbo = 0;
            }
            if (vao != 0)
            {
                glDeleteVertexArrays(1, &vao);
                vao = 0;
            }
            if (program != 0)
            {
                glDeleteProgram(program);
                program = 0;
            }
        }

        void begin(int width, int height)
        {
            fb_width = std::max(width, 1);
            fb_height = std::max(height, 1);
            vertices.clear();
        }

        void add_rect(float x, float y, float w, float h, UiColor c)
        {
            if (w <= 0.0f || h <= 0.0f)
            {
                return;
            }

            add_triangle(x, y, x + w, y, x + w, y + h, c);
            add_triangle(x, y, x + w, y + h, x, y + h, c);
        }

        void add_outline_rect(float x, float y, float w, float h, float thickness, UiColor c)
        {
            if (thickness <= 0.0f || w <= 0.0f || h <= 0.0f)
            {
                return;
            }

            add_rect(x, y, w, thickness, c);
            add_rect(x, y + h - thickness, w, thickness, c);
            add_rect(x, y + thickness, thickness, h - 2.0f * thickness, c);
            add_rect(x + w - thickness, y + thickness, thickness, h - 2.0f * thickness, c);
        }

        void add_text(float x, float y, float scale, UiColor color, const std::string& text)
        {
            float cursor_x = x;
            for (char ch : text)
            {
                if (ch == '\n')
                {
                    cursor_x = x;
                    y += glyph_line_height * scale;
                    continue;
                }

                draw_char(cursor_x, y, scale, color, ch);
                cursor_x += glyph_character_advance * scale;
            }
        }

        void end()
        {
            if (vertices.empty() || program == 0)
            {
                return;
            }

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);

            glUseProgram(program);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER,
                         GLsizeiptr(vertices.size() * sizeof(UiVertex)),
                         vertices.data(),
                         GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertices.size()));

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }

    private:
        static bool check_shader_compile(GLuint shader)
        {
            GLint ok = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
            if (!ok)
            {
                char info[shader_log_buffer_size];
                glGetShaderInfoLog(shader, GLsizei(sizeof(info)), nullptr, info);
                std::cerr << "UI shader compile error:\n" << info << "\n";
                return false;
            }
            return true;
        }

        void add_triangle(float x0, float y0, float x1, float y1, float x2, float y2, UiColor c)
        {
            vertices.push_back(to_vertex(x0, y0, c));
            vertices.push_back(to_vertex(x1, y1, c));
            vertices.push_back(to_vertex(x2, y2, c));
        }

        UiVertex to_vertex(float px, float py, UiColor c) const
        {
            float nx = (2.0f * px / float(fb_width)) - 1.0f;
            float ny = 1.0f - (2.0f * py / float(fb_height));
            return UiVertex{ nx, ny, c.r, c.g, c.b, c.a };
        }

        static std::array<unsigned char, 7> glyph(char ch)
        {
            switch (ch)
            {
                case 'A': return { 14, 17, 17, 31, 17, 17, 17 };
                case 'B': return { 30, 17, 17, 30, 17, 17, 30 };
                case 'C': return { 14, 17, 16, 16, 16, 17, 14 };
                case 'D': return { 30, 17, 17, 17, 17, 17, 30 };
                case 'E': return { 31, 16, 16, 30, 16, 16, 31 };
                case 'F': return { 31, 16, 16, 30, 16, 16, 16 };
                case 'G': return { 14, 17, 16, 23, 17, 17, 14 };
                case 'H': return { 17, 17, 17, 31, 17, 17, 17 };
                case 'I': return { 14, 4, 4, 4, 4, 4, 14 };
                case 'J': return { 1, 1, 1, 1, 1, 17, 14 };
                case 'K': return { 17, 18, 20, 24, 20, 18, 17 };
                case 'L': return { 16, 16, 16, 16, 16, 16, 31 };
                case 'M': return { 17, 27, 21, 21, 17, 17, 17 };
                case 'N': return { 17, 25, 21, 19, 17, 17, 17 };
                case 'O': return { 14, 17, 17, 17, 17, 17, 14 };
                case 'P': return { 30, 17, 17, 30, 16, 16, 16 };
                case 'Q': return { 14, 17, 17, 17, 21, 18, 13 };
                case 'R': return { 30, 17, 17, 30, 20, 18, 17 };
                case 'S': return { 14, 17, 16, 14, 1, 17, 14 };
                case 'T': return { 31, 4, 4, 4, 4, 4, 4 };
                case 'U': return { 17, 17, 17, 17, 17, 17, 14 };
                case 'V': return { 17, 17, 17, 17, 17, 10, 4 };
                case 'W': return { 17, 17, 17, 21, 21, 21, 10 };
                case 'X': return { 17, 17, 10, 4, 10, 17, 17 };
                case 'Y': return { 17, 17, 10, 4, 4, 4, 4 };
                case 'Z': return { 31, 1, 2, 4, 8, 16, 31 };
                case '0': return { 14, 17, 19, 21, 25, 17, 14 };
                case '1': return { 4, 12, 4, 4, 4, 4, 14 };
                case '2': return { 14, 17, 1, 2, 4, 8, 31 };
                case '3': return { 14, 17, 1, 6, 1, 17, 14 };
                case '4': return { 2, 6, 10, 18, 31, 2, 2 };
                case '5': return { 31, 16, 30, 1, 1, 17, 14 };
                case '6': return { 6, 8, 16, 30, 17, 17, 14 };
                case '7': return { 31, 1, 2, 4, 8, 8, 8 };
                case '8': return { 14, 17, 17, 14, 17, 17, 14 };
                case '9': return { 14, 17, 17, 15, 1, 2, 12 };
                case ':': return { 0, 4, 4, 0, 4, 4, 0 };
                case '.': return { 0, 0, 0, 0, 0, 6, 6 };
                case ',': return { 0, 0, 0, 0, 6, 6, 4 };
                case '-': return { 0, 0, 0, 31, 0, 0, 0 };
                case '/': return { 1, 2, 4, 8, 16, 0, 0 };
                case '(': return { 2, 4, 8, 8, 8, 4, 2 };
                case ')': return { 8, 4, 2, 2, 2, 4, 8 };
                case '=': return { 0, 31, 0, 31, 0, 0, 0 };
                case '>': return { 16, 8, 4, 2, 4, 8, 16 };
                case '<': return { 1, 2, 4, 8, 4, 2, 1 };
                case '_': return { 0, 0, 0, 0, 0, 0, 31 };
                case ' ': return { 0, 0, 0, 0, 0, 0, 0 };
                default: return { 31, 17, 1, 2, 4, 0, 4 };
            }
        }

        void draw_char(float x, float y, float scale, UiColor color, char ch)
        {
            char up = ch;
            if (up >= 'a' && up <= 'z')
            {
                up = char(up - 'a' + 'A');
            }

            const std::array<unsigned char, 7> g = glyph(up);
            for (int row = 0; row < glyph_rows; ++row)
            {
                unsigned char bits = g[std::size_t(row)];
                for (int col = 0; col < glyph_columns; ++col)
                {
                    if ((bits & (1 << ((glyph_columns - 1) - col))) != 0)
                    {
                        add_rect(x + float(col) * scale,
                                 y + float(row) * scale,
                                 scale,
                                 scale,
                                 color);
                    }
                }
            }
        }

    private:
        GLuint program = 0;
        GLuint vao = 0;
        GLuint vbo = 0;
        int fb_width = 1;
        int fb_height = 1;
        std::vector<UiVertex> vertices;
    };

    static void glfw_err(int code, const char* desc)
    {
        std::cerr << "GLFW error (" << code << "): " << (desc ? desc : "") << "\n";
    }

    static UiRenderer g_ui_renderer;

    static const char* terrain_mode_label(Terrain::Mode mode)
    {
        if (mode == Terrain::Mode::Sinusoidal)
        {
            return "Sinusoidal";
        }
        if (mode == Terrain::Mode::DiamondSquare)
        {
            return "DiamondSquare";
        }
        if (mode == Terrain::Mode::Perlin)
        {
            return "Perlin";
        }
        return "Simplex";
    }

    static const char* color_scheme_label(Terrain::ColorScheme scheme)
    {
        switch (scheme)
        {
            case Terrain::ColorScheme::Canyon: return "Canyon";
            case Terrain::ColorScheme::FrozenTundra: return "Tundra";
            case Terrain::ColorScheme::Grayscale: return "Gray";
            case Terrain::ColorScheme::Volcanic: return "Volcanic";
        }
        return "Unknown";
    }

    static float normalize_to_01(float value, float min_value, float max_value)
    {
        if (max_value <= min_value)
        {
            return 0.0f;
        }
        return clampf((value - min_value) / (max_value - min_value), 0.0f, 1.0f);
    }

    static bool point_in_rect(float px, float py, float x, float y, float w, float h)
    {
        return px >= x && px <= (x + w) && py >= y && py <= (y + h);
    }
}

App::App()
    : window(nullptr)
    , fb_width(default_window_width)
    , fb_height(default_window_height)
    , max_dt(max_frame_dt)
    , wireframe_enabled(false)
    , show_help_overlay(true)
    , show_terrain_panel(false)
    , active_slider_id(-1)
    , value_edit_active(false)
    , edited_value_slider_id(-1)
    , value_edit_buffer()
{
}

bool App::init()
{
    glfwSetErrorCallback(glfw_err);

    if (!glfwInit())
    {
        std::cerr << "glfwInit failed\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(default_window_width, default_window_height, app_title, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "glfwCreateWindow failed\n";
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(window, this);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoaderLoadGL())
    {
        std::cerr << "gladLoaderLoadGL failed\n";
        return false;
    }

    std::cout << "Vendor  : " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Version : " << glGetString(GL_VERSION) << "\n";

    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);

    glfwSetFramebufferSizeCallback(window, App::framebuffer_size_callback);
    glfwSetKeyCallback(window, App::key_callback);
    glfwSetMouseButtonCallback(window, App::mouse_button_callback);
    glfwSetCursorPosCallback(window, App::mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);

    if (!scene.init())
    {
        return false;
    }

    if (!g_ui_renderer.init())
    {
        return false;
    }

    return true;
}

void App::run()
{
    auto set_terrain_panel_open = [this](bool panel_open) {
        show_terrain_panel = panel_open;
        active_slider_id = -1;
        value_edit_active = false;
        edited_value_slider_id = -1;
        value_edit_buffer.clear();
        glfwSetInputMode(window, GLFW_CURSOR, panel_open ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        input.reset_mouse_delta_tracking();
    };

    double last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        float dt = float(now - last_time);
        last_time = now;
        dt = clampf(dt, 0.0f, max_dt);

        input.begin_frame();
        glfwPollEvents();

        if (value_edit_active && input.is_key_pressed(GLFW_KEY_ESCAPE))
        {
            value_edit_active = false;
            edited_value_slider_id = -1;
            value_edit_buffer.clear();
        }
        else if (input.is_key_pressed(GLFW_KEY_ESCAPE))
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        if (!value_edit_active)
        {
            if (input.is_key_pressed(GLFW_KEY_F1))
            {
                show_help_overlay = !show_help_overlay;
            }

            if (input.is_key_pressed(GLFW_KEY_TAB))
            {
                set_terrain_panel_open(!show_terrain_panel);
            }

            if (input.is_key_pressed(GLFW_KEY_F3))
            {
                wireframe_enabled = !wireframe_enabled;
            }
            if (input.is_key_pressed(GLFW_KEY_F2))
            {
                camera.toggle_movement_mode();
            }

            if (input.is_key_pressed(GLFW_KEY_1))
            {
                scene.set_terrain_mode(Terrain::Mode::Sinusoidal);
            }

            if (input.is_key_pressed(GLFW_KEY_2))
            {
                scene.set_terrain_mode(Terrain::Mode::DiamondSquare);
            }

            if (input.is_key_pressed(GLFW_KEY_3))
            {
                scene.set_terrain_mode(Terrain::Mode::Perlin);
            }

            if (input.is_key_pressed(GLFW_KEY_4))
            {
                scene.set_terrain_mode(Terrain::Mode::Simplex);
            }

            if (input.is_key_pressed(GLFW_KEY_R))
            {
                scene.get_terrain_mut().reset_active_mode_params_to_defaults();
            }

            if (input.is_key_pressed(GLFW_KEY_C))
            {
                Terrain& active_terrain = scene.get_terrain_mut();
                int next_scheme = (static_cast<int>(active_terrain.get_color_scheme()) + 1) % 4;
                active_terrain.set_color_scheme(static_cast<Terrain::ColorScheme>(next_scheme));
            }

        }

        Terrain& terrain = scene.get_terrain_mut();

        camera.update(input, dt, scene.get_terrain(), !show_terrain_panel);
        scene.update(camera);

        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.render(now, fb_width, fb_height, camera, wireframe_enabled);

        const float mouse_x = float(input.get_mouse_x());
        const float mouse_y = float(input.get_mouse_y());
        const bool mouse_down = input.is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT);
        const bool mouse_pressed = input.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
        const bool mouse_released = input.is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT);

        g_ui_renderer.begin(fb_width, fb_height);

        if (show_help_overlay)
        {
            const float ox = help_overlay_x;
            const float oy = help_overlay_y;
            g_ui_renderer.add_rect(ox, oy, help_overlay_width, help_overlay_height, ui_color_help_overlay_bg);
            g_ui_renderer.add_outline_rect(ox, oy, help_overlay_width, help_overlay_height, help_overlay_outline_thickness, ui_color_help_overlay_outline);
            g_ui_renderer.add_text(ox + help_overlay_text_x_offset, oy + help_overlay_text_y_start, help_overlay_text_scale, ui_color_help_overlay_text, "1 sinusoidal");
            g_ui_renderer.add_text(ox + 12.0f, oy + help_overlay_text_y_start + help_overlay_text_y_step * 1.0f, help_overlay_text_scale, ui_color_help_overlay_text, "2 diamond square");
            g_ui_renderer.add_text(ox + 12.0f, oy + help_overlay_text_y_start + help_overlay_text_y_step * 2.0f, help_overlay_text_scale, ui_color_help_overlay_text, "3 perlin");
            g_ui_renderer.add_text(ox + 12.0f, oy + help_overlay_text_y_start + help_overlay_text_y_step * 3.0f, help_overlay_text_scale, ui_color_help_overlay_text, "4 simplex");
            g_ui_renderer.add_text(ox + 12.0f, oy + help_overlay_text_y_start + help_overlay_text_y_step * 4.0f, help_overlay_text_scale, ui_color_help_overlay_text, "tab terrain control");
            g_ui_renderer.add_text(ox + 12.0f, oy + help_overlay_text_y_start + help_overlay_text_y_step * 5.0f, help_overlay_text_scale, ui_color_help_overlay_text, "f1 help");
            g_ui_renderer.add_text(ox + 12.0f, oy + help_overlay_text_y_start + help_overlay_text_y_step * 6.0f, help_overlay_text_scale, ui_color_help_overlay_text, "f2 movement mode");
            g_ui_renderer.add_text(ox + 12.0f, oy + help_overlay_text_y_start + help_overlay_text_y_step * 7.0f, help_overlay_text_scale, ui_color_help_overlay_text, "f3 wireframe");
            g_ui_renderer.add_text(ox + 12.0f, oy + help_overlay_text_y_start + help_overlay_text_y_step * 8.0f, help_overlay_text_scale, ui_color_help_overlay_text, "c color palette");
        }

        if (show_terrain_panel)
        {
            const float px = terrain_panel_x;
            const float py = terrain_panel_y;
            const float panel_width = terrain_panel_width;
            int slider_row_count = 4;
            if (terrain.get_mode() == Terrain::Mode::Sinusoidal)
            {
                slider_row_count = 3;
            }
            else if (terrain.get_mode() == Terrain::Mode::Perlin || terrain.get_mode() == Terrain::Mode::Simplex)
            {
                slider_row_count = 7;
            }

            const float panel_height = terrain_panel_top_padding
                                     + terrain_panel_header_height
                                     + float(slider_row_count) * terrain_slider_row_step
                                     + terrain_panel_button_gap
                                     + terrain_button_height
                                     + terrain_panel_bottom_padding
                                     + 72.0f; // Space for palette selector

            g_ui_renderer.add_rect(px, py, panel_width, panel_height, ui_color_panel_bg);
            g_ui_renderer.add_outline_rect(px, py, panel_width, panel_height, terrain_panel_outline_thickness, ui_color_panel_outline);
            g_ui_renderer.add_text(px + terrain_panel_title_x_offset, py + terrain_panel_title_y_offset, terrain_panel_title_scale, ui_color_panel_title, "TERRAIN PANEL");

            std::string mode_line = std::string("ACTIVE MODE: ") + terrain_mode_label(terrain.get_mode());
            g_ui_renderer.add_text(px + terrain_panel_title_x_offset, py + terrain_panel_mode_y_offset, terrain_panel_mode_scale, ui_color_panel_text, mode_line);

            int next_slider_id = 0;
            auto stop_value_edit = [&]() {
                value_edit_active = false;
                edited_value_slider_id = -1;
                value_edit_buffer.clear();
            };

            auto begin_value_edit = [&](int slider_id, float current_value, bool as_integer) {
                value_edit_active = true;
                edited_value_slider_id = slider_id;
                char value_text[64];
                if (as_integer)
                {
                    std::snprintf(value_text, sizeof(value_text), "%d", int(std::round(current_value)));
                }
                else
                {
                    std::snprintf(value_text, sizeof(value_text), "%.3f", current_value);
                }
                value_edit_buffer = value_text;
            };

            auto try_commit_value_edit = [&](bool as_integer, float min_value, float max_value, float* out_value) -> bool {
                if (value_edit_buffer.empty())
                {
                    stop_value_edit();
                    return false;
                }

                char* parse_end = nullptr;
                double parsed_value = std::strtod(value_edit_buffer.c_str(), &parse_end);
                if (parse_end == value_edit_buffer.c_str() || *parse_end != '\0')
                {
                    stop_value_edit();
                    return false;
                }

                float new_value = clampf(float(parsed_value), min_value, max_value);
                if (as_integer)
                {
                    new_value = std::round(new_value);
                }
                *out_value = new_value;
                stop_value_edit();
                return true;
            };

            auto append_pressed_digit = [&](std::string& buffer) {
                for (int digit = 0; digit <= 9; ++digit)
                {
                    if (input.is_key_pressed(GLFW_KEY_0 + digit) || input.is_key_pressed(GLFW_KEY_KP_0 + digit))
                    {
                        buffer.push_back(char('0' + digit));
                    }
                }
            };

            auto draw_button = [&](float x, float y, float w, float h, const char* label) -> bool {
                bool hovered = point_in_rect(mouse_x, mouse_y, x, y, w, h);
                bool pressed = hovered && mouse_down;
                UiColor fill = ui_color_button_default;
                if (hovered)
                {
                    fill = ui_color_button_hover;
                }
                if (pressed)
                {
                    fill = ui_color_button_pressed;
                }
                g_ui_renderer.add_rect(x, y, w, h, fill);
                g_ui_renderer.add_outline_rect(x, y, w, h, terrain_panel_outline_thickness, ui_color_button_outline);
                g_ui_renderer.add_text(x + slider_row_x_offset, y + slider_label_y_offset, slider_label_scale, ui_color_button_text, label);
                return hovered && mouse_pressed;
            };

            auto draw_slider_row = [&](float y,
                                       const char* label,
                                       float value,
                                       float min_value,
                                       float max_value,
                                       bool as_integer,
                                       float* out_value) -> bool {
                const int slider_id = next_slider_id++;
                const float row_x = px + slider_row_x_offset;
                const float row_w = panel_width - slider_row_width_inset;
                const float label_scale = slider_label_scale;
                const float label_x = row_x + slider_label_x_offset;
                const float label_y = y + slider_label_y_offset;
                const float value_box_w = slider_value_box_width;
                const float value_box_h = slider_value_box_height;
                const float right_padding = slider_right_padding;
                const float gap_after_label = slider_gap_after_label;
                const float gap_after_value_box = slider_gap_after_value_box;
                const float label_w = float(std::string(label).size()) * 6.0f * label_scale;
                float value_box_x = label_x + label_w + gap_after_label;
                const float value_box_y = y + slider_value_box_y_offset;
                const float max_value_box_x = row_x + row_w - right_padding - value_box_w;
                if (value_box_x > max_value_box_x)
                {
                    value_box_x = max_value_box_x;
                }
                const float track_x = value_box_x + value_box_w + gap_after_value_box;
                const float track_w = (row_x + row_w - right_padding) - track_x;
                const float track_y = y + slider_track_y_offset;
                const float track_h = slider_track_height;
                const float knob_w = slider_knob_width;
                const float knob_h = slider_knob_height;

                float t = normalize_to_01(value, min_value, max_value);
                float knob_x = track_x + t * (track_w - knob_w);
                float knob_y = track_y - slider_knob_y_offset;

                bool hover_track = point_in_rect(mouse_x, mouse_y, track_x, track_y - slider_track_hit_y_offset, track_w, track_h + slider_track_hit_extra_height);
                bool hover_knob = point_in_rect(mouse_x, mouse_y, knob_x, knob_y, knob_w, knob_h);
                bool hover_value_box = point_in_rect(mouse_x, mouse_y, value_box_x, value_box_y, value_box_w, value_box_h);
                bool hover_any = hover_track || hover_knob;
                bool is_editing_this_value = value_edit_active && edited_value_slider_id == slider_id;

                if (mouse_pressed && hover_value_box)
                {
                    if (!is_editing_this_value)
                    {
                        begin_value_edit(slider_id, value, as_integer);
                        is_editing_this_value = true;
                    }
                    active_slider_id = -1;
                }

                if (!value_edit_active && mouse_pressed && hover_any && (active_slider_id < 0 || active_slider_id == slider_id))
                {
                    active_slider_id = slider_id;
                }
                if (mouse_released && active_slider_id == slider_id)
                {
                    active_slider_id = -1;
                }

                bool active = active_slider_id == slider_id;

                g_ui_renderer.add_rect(row_x, y, row_w, slider_row_height, active ? ui_color_slider_row_active
                                                                  : ui_color_slider_row_inactive);
                g_ui_renderer.add_text(label_x, label_y, label_scale, ui_color_slider_label, label);

                char value_text[64];
                if (as_integer)
                {
                    std::snprintf(value_text, sizeof(value_text), "%d", int(std::round(value)));
                }
                else
                {
                    std::snprintf(value_text, sizeof(value_text), "%.3f", value);
                }
                UiColor value_box_fill = ui_color_value_box_default;
                if (hover_value_box)
                {
                    value_box_fill = ui_color_value_box_hover;
                }
                if (is_editing_this_value)
                {
                    value_box_fill = ui_color_value_box_editing;
                }
                g_ui_renderer.add_rect(value_box_x, value_box_y, value_box_w, value_box_h, value_box_fill);
                g_ui_renderer.add_outline_rect(value_box_x, value_box_y, value_box_w, value_box_h, slider_outline_thickness, ui_color_button_outline);
                const char* shown_value_text = is_editing_this_value ? value_edit_buffer.c_str() : value_text;
                g_ui_renderer.add_text(value_box_x + slider_value_text_x_offset, y + slider_value_text_y_offset, slider_label_scale, ui_color_value_text, shown_value_text);

                g_ui_renderer.add_rect(track_x, track_y, track_w, track_h, ui_color_slider_track_bg);
                g_ui_renderer.add_rect(track_x, track_y, t * track_w, track_h, ui_color_slider_track_fill);

                UiColor knob_color = ui_color_knob_default;
                if (hover_any)
                {
                    knob_color = ui_color_knob_hover;
                }
                if (active)
                {
                    knob_color = ui_color_knob_active;
                }
                g_ui_renderer.add_rect(knob_x, knob_y, knob_w, knob_h, knob_color);
                g_ui_renderer.add_outline_rect(knob_x, knob_y, knob_w, knob_h, slider_outline_thickness, ui_color_knob_outline);

                bool value_committed = false;
                if (is_editing_this_value)
                {
                    if (input.is_key_pressed(GLFW_KEY_BACKSPACE) && !value_edit_buffer.empty())
                    {
                        value_edit_buffer.pop_back();
                    }

                    append_pressed_digit(value_edit_buffer);

                    if (!as_integer && (input.is_key_pressed(GLFW_KEY_PERIOD) || input.is_key_pressed(GLFW_KEY_KP_DECIMAL)))
                    {
                        if (value_edit_buffer.find('.') == std::string::npos)
                        {
                            if (value_edit_buffer.empty())
                            {
                                value_edit_buffer.push_back('0');
                            }
                            value_edit_buffer.push_back('.');
                        }
                    }

                    const bool allow_negative = min_value < 0.0f;
                    if (allow_negative && (input.is_key_pressed(GLFW_KEY_MINUS) || input.is_key_pressed(GLFW_KEY_KP_SUBTRACT)))
                    {
                        if (value_edit_buffer.empty())
                        {
                            value_edit_buffer.push_back('-');
                        }
                    }

                    if (input.is_key_pressed(GLFW_KEY_ENTER) || input.is_key_pressed(GLFW_KEY_KP_ENTER))
                    {
                        value_committed = try_commit_value_edit(as_integer, min_value, max_value, out_value);
                    }
                    else if (input.is_key_pressed(GLFW_KEY_ESCAPE))
                    {
                        stop_value_edit();
                    }
                    else if (mouse_pressed && !hover_value_box)
                    {
                        value_committed = try_commit_value_edit(as_integer, min_value, max_value, out_value);
                    }
                }

                if (active && mouse_down)
                {
                    float slider_t = clampf((mouse_x - track_x) / track_w, 0.0f, 1.0f);
                    float new_value = min_value + slider_t * (max_value - min_value);
                    if (as_integer)
                    {
                        new_value = std::round(new_value);
                    }
                    *out_value = new_value;
                }

                return value_committed || (active && mouse_down);
            };

            bool changed = false;
            const float row_start = py + terrain_panel_top_padding + terrain_panel_header_height;
            const float row_step = terrain_slider_row_step;

            if (terrain.get_mode() == Terrain::Mode::Sinusoidal)
            {
                Terrain::SinusoidalParams params = terrain.get_sinusoidal_params();
                float value = params.amplitude;
                changed = draw_slider_row(row_start + row_step * 0.0f, "Amplitude", params.amplitude, sinusoidal_amplitude_range.min_value, sinusoidal_amplitude_range.max_value, false, &value) || changed;
                params.amplitude = value;

                value = params.x_frequency;
                changed = draw_slider_row(row_start + row_step * 1.0f, "X Frequency", params.x_frequency, 0.01f, 1.0f, false, &value) || changed;
                params.x_frequency = value;

                value = params.z_frequency;
                changed = draw_slider_row(row_start + row_step * 2.0f, "Z Frequency", params.z_frequency, 0.01f, 1.0f, false, &value) || changed;
                params.z_frequency = value;

                if (changed)
                {
                    terrain.set_sinusoidal_params(params);
                }
            }
            else if (terrain.get_mode() == Terrain::Mode::Perlin)
            {
                Terrain::PerlinParams params = terrain.get_perlin_params();

                float value = params.height_scale;
                changed = draw_slider_row(row_start + row_step * 0.0f, "Height Scale", params.height_scale, fractal_height_scale_range.min_value, fractal_height_scale_range.max_value, false, &value) || changed;
                params.height_scale = value;

                value = params.base_frequency;
                changed = draw_slider_row(row_start + row_step * 1.0f, "Base Frequency", params.base_frequency, fractal_base_frequency_range.min_value, fractal_base_frequency_range.max_value, false, &value) || changed;
                params.base_frequency = value;

                value = float(params.octaves);
                changed = draw_slider_row(row_start + row_step * 2.0f, "Octaves", float(params.octaves), fractal_octaves_range.min_value, fractal_octaves_range.max_value, true, &value) || changed;
                params.octaves = int(value);

                value = params.persistence;
                changed = draw_slider_row(row_start + row_step * 3.0f, "Persistence", params.persistence, fractal_persistence_range.min_value, fractal_persistence_range.max_value, false, &value) || changed;
                params.persistence = value;

                value = params.lacunarity;
                changed = draw_slider_row(row_start + row_step * 4.0f, "Lacunarity", params.lacunarity, fractal_lacunarity_range.min_value, fractal_lacunarity_range.max_value, false, &value) || changed;
                params.lacunarity = value;

                value = params.redistribution_power;
                changed = draw_slider_row(row_start + row_step * 5.0f,
                                          "Redistribution",
                                          params.redistribution_power,
                                          fractal_redistribution_range.min_value,
                                          fractal_redistribution_range.max_value,
                                          false,
                                          &value) || changed;
                params.redistribution_power = value;

                value = float(params.seed);
                changed = draw_slider_row(row_start + row_step * 6.0f, "Seed", float(params.seed), fractal_seed_range.min_value, fractal_seed_range.max_value, true, &value) || changed;
                params.seed = int(value);

                if (changed)
                {
                    terrain.set_perlin_params(params);
                }
            }
            else if (terrain.get_mode() == Terrain::Mode::Simplex)
            {
                Terrain::SimplexParams params = terrain.get_simplex_params();

                float value = params.height_scale;
                changed = draw_slider_row(row_start + row_step * 0.0f, "Height Scale", params.height_scale, fractal_height_scale_range.min_value, fractal_height_scale_range.max_value, false, &value) || changed;
                params.height_scale = value;

                value = params.base_frequency;
                changed = draw_slider_row(row_start + row_step * 1.0f, "Base Frequency", params.base_frequency, fractal_base_frequency_range.min_value, fractal_base_frequency_range.max_value, false, &value) || changed;
                params.base_frequency = value;

                value = float(params.octaves);
                changed = draw_slider_row(row_start + row_step * 2.0f, "Octaves", float(params.octaves), fractal_octaves_range.min_value, fractal_octaves_range.max_value, true, &value) || changed;
                params.octaves = int(value);

                value = params.persistence;
                changed = draw_slider_row(row_start + row_step * 3.0f, "Persistence", params.persistence, fractal_persistence_range.min_value, fractal_persistence_range.max_value, false, &value) || changed;
                params.persistence = value;

                value = params.lacunarity;
                changed = draw_slider_row(row_start + row_step * 4.0f, "Lacunarity", params.lacunarity, fractal_lacunarity_range.min_value, fractal_lacunarity_range.max_value, false, &value) || changed;
                params.lacunarity = value;

                value = params.redistribution_power;
                changed = draw_slider_row(row_start + row_step * 5.0f,
                                          "Redistribution",
                                          params.redistribution_power,
                                          fractal_redistribution_range.min_value,
                                          fractal_redistribution_range.max_value,
                                          false,
                                          &value) || changed;
                params.redistribution_power = value;

                value = float(params.seed);
                changed = draw_slider_row(row_start + row_step * 6.0f, "Seed", float(params.seed), fractal_seed_range.min_value, fractal_seed_range.max_value, true, &value) || changed;
                params.seed = int(value);

                if (changed)
                {
                    terrain.set_simplex_params(params);
                }
            }
            else
            {
                Terrain::DiamondSquareParams params = terrain.get_diamond_square_params();

                float value = params.initial_amplitude;
                changed = draw_slider_row(row_start + row_step * 0.0f,
                                          "Initial Amplitude",
                                          params.initial_amplitude,
                                          diamond_square_initial_amplitude_range.min_value,
                                          diamond_square_initial_amplitude_range.max_value,
                                          false,
                                          &value) || changed;
                params.initial_amplitude = value;

                value = params.roughness;
                changed = draw_slider_row(row_start + row_step * 1.0f, "Roughness", params.roughness, diamond_square_roughness_range.min_value, diamond_square_roughness_range.max_value, false, &value) || changed;
                params.roughness = value;

                value = float(params.seed);
                changed = draw_slider_row(row_start + row_step * 2.0f, "Seed", float(params.seed), diamond_square_seed_range.min_value, diamond_square_seed_range.max_value, true, &value) || changed;
                params.seed = int(value);

                value = float(params.exponent);
                changed = draw_slider_row(row_start + row_step * 3.0f, "Exponent", float(params.exponent), diamond_square_exponent_range.min_value, diamond_square_exponent_range.max_value, true, &value) || changed;
                params.exponent = int(value);

                if (changed)
                {
                    terrain.set_diamond_square_params(params);
                }
            }

            float button_y = row_start + float(slider_row_count) * row_step + terrain_panel_button_gap;
            if (draw_button(px + terrain_button_reset_x_offset, button_y, terrain_button_reset_width, terrain_button_height, "Reset Defaults"))
            {
                terrain.reset_active_mode_params_to_defaults();
            }

            // Color selection changes a shader uniform, not terrain geometry.
            g_ui_renderer.add_text(px + 12.0f, button_y + 46.0f, 1.5f, ui_color_panel_text, "COLOR PALETTE");
            const Terrain::ColorScheme schemes[] = {
                Terrain::ColorScheme::Grayscale,
                Terrain::ColorScheme::Canyon,
                Terrain::ColorScheme::FrozenTundra,
                Terrain::ColorScheme::Volcanic
            };
            const float palette_button_width = 118.0f;
            const float palette_button_gap = 6.0f;
            const float palette_button_y = button_y + 64.0f;
            for (int i = 0; i < 4; ++i)
            {
                const float bx = px + 12.0f + float(i) * (palette_button_width + palette_button_gap);
                if (draw_button(bx, palette_button_y, palette_button_width, 32.0f, color_scheme_label(schemes[i])))
                {
                    terrain.set_color_scheme(schemes[i]);
                }
                if (terrain.get_color_scheme() == schemes[i])
                {
                    g_ui_renderer.add_outline_rect(bx, palette_button_y, palette_button_width, 32.0f, 2.0f, ui_color_slider_track_fill);
                }
            }
        }

        g_ui_renderer.end();

        glfwSwapBuffers(window);
        input.end_frame();
    }
}

void App::shutdown()
{
    g_ui_renderer.shutdown();
    scene.shutdown();

    if (window)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

void App::on_framebuffer_resize(int width, int height)
{
    fb_width = width;
    fb_height = height;
    glViewport(0, 0, fb_width, fb_height);
}

void App::on_key(int key, int action)
{
    input.set_key(key, action);
}

void App::on_mouse_button(int button, int action)
{
    input.set_mouse_button(button, action);
}

void App::on_mouse_move(double xpos, double ypos)
{
    input.set_mouse_position(xpos, ypos);
}

void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    App* app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->on_framebuffer_resize(width, height);
    }
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    App* app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->on_key(key, action);
    }
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    (void)mods;

    App* app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->on_mouse_button(button, action);
    }
}

void App::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    App* app = (App*)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->on_mouse_move(xpos, ypos);
    }
}
