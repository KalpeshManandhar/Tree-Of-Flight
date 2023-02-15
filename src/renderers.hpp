#pragma once
#include "imgui.h"
#include <glm.hpp>
#include <GLFW/glfw3.h>
#include <functional>

//Some color values
namespace Color {
#define col constexpr glm::vec3

    col white = { 1.0,1.0,1.0 };
    col silver = { 0.75,0.75,0.75 };
    col gray = { 0.5,0.5,0.5 };
    col black = { 0.0,0.0,0.0 };
    col red = { 1.0,0.0,0.0 };
    col maroon = { 0.5,0.0,0.0 };
    col yellow = { 1.0,1.0,0.0 };
    col olive = { 0.5,0.5,0.0 };
    col lime = { 0.0,1.0,0.0 };
    col green = { 0.0,0.5,0.0 };
    col aqua = { 0.0,1.0,1.0 };
    col teal = { 0.0,0.5,0.5 };
    col blue = { 0.0,0.0,1.0 };
    col navy = { 0.0,0.0,0.5 };
    col fuchsia = { 1.0,0.0,1.0 };
    col purple = { 0.5,0.0,0.5 };
#undef col
}

namespace Context {
    extern GLFWwindow* window;

    //Callback std::function objects
    extern std::function<void(int button, int action, int mods)> mouse_click_callback;
    extern std::function<void(int button, int action, int mods)> keyboard_callback;
    extern std::function<void(double delx, double dely)> cursor_move_callback;
    extern std::function<void(double xoff, double yoff)> scroll_callback;
    glm::vec2 get_real_dim();
    struct Texture {
        unsigned int tex;
        int width;
        int height;
        int channels;
    };
    extern Texture default_texture;



    bool createTexture(std::string img_file, Texture& tex);
    int init();
    void clean();

    struct Circle {
        glm::vec2 center;
        float radius;
        glm::vec3 color;
        glm::vec2 scale = { 1.f,1.f };
        float rotate = 0.f;

        void draw(Texture tex= default_texture, bool reuse_shader = false)const;
    };
    struct Triangle {
        glm::vec2 pos1;
        glm::vec2 pos2;
        glm::vec2 pos3;
        glm::vec3 color;

        void draw(Texture tex= default_texture, bool reuse_shader = false)const;
    };

    struct Rectangle {
        glm::vec2 center;
        glm::vec2 size;
        glm::vec3 color;
        glm::vec2 scale = { 1.f,1.f };
        float rotate = 0.f;

        void draw(Texture tex = default_texture, bool reuse_shader = false)const;
    };
    struct Line {
        glm::vec2 pos1;
        glm::vec2 pos2;
        glm::vec3 color;
        float line_width = 1;

        void draw(Texture tex = default_texture, bool reuse_shader = false);
    };

    glm::vec2 get_mouse_pos();

    void set_mouse_pos(glm::vec2 pos);
    bool is_mouse_button_pressed(int mouse_button);

    void init_rendering(glm::vec3 clear_col);
    bool poll_events_and_decide_quit();
    void finish_rendering();
}