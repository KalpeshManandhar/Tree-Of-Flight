#pragma once
#include "imgui.h"
#include <glm.hpp>
#include <GLFW/glfw3.h>
#include <functional>
#include <stb_image.h>
#include <chrono>


//Some color values
namespace Color {
#define NRGB(r,g,b) {(r/255.0f),(g/255.0f),(b/255.0f)}

#define col constexpr glm::vec3
    // reddish 
    col red2 = NRGB(239,17,42);
    col crimson = NRGB(216,0,50);
    col imperialRed = NRGB(237,71,74);
    col redWood = NRGB(162,62,72);
    // orange-ish
    col orange2 = NRGB(253, 121, 121);
    col lightCoral = NRGB(240,106,109);
    col tangerine = NRGB(229,143,101);
    col pale = NRGB(240, 150, 102);
    // greenish
    col pgmtGreen = NRGB(38,166,55);
    col celadon = NRGB(152,223,175);
    col emerald = NRGB(83,202,121);
    // greenish black
    col blackOlive = NRGB(63,71,57);

    col jetblack = NRGB(44,48,46);


    col white = { 1.0,1.0,1.0 };
    col silver = { 0.75,0.75,0.75 };
    col gray = { 0.5,0.5,0.5 };
    // col black = { 0.0,0.0,0.0 };
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

class Timer {
private:
    // Type aliases to make accessing nested type easier
    using Clock = std::chrono::steady_clock;
    using Second = std::chrono::duration<double, std::ratio<1> >;

    std::chrono::time_point<Clock> m_beg{ Clock::now() };

public:
    void reset() {
        m_beg = Clock::now();
    }

    double elapsed() const {
        return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
    }
};

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

    void set_window_size(int width, int height);
    void set_window_title(std::string title);
    void set_window_icon(std::string icon_file);
    glm::vec2 get_mouse_pos();
    void set_mouse_pos(glm::vec2 pos);
    bool is_mouse_button_pressed(int mouse_button);
    bool is_key_pressed(int scancode);

    void init_rendering(glm::vec3 clear_col);
    bool poll_events_and_decide_quit();
    void set_close_window();
    void finish_rendering();
}