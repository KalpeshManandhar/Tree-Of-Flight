#include "glad/glad.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib>
#include "renderers.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace Context {
    //Some context data
    GLFWwindow* window = nullptr;
    static unsigned int width = 1080;
    static unsigned int height = 766;
    static glm::vec2 mouse_pos = { 0,0 };

    glm::vec2 get_real_dim() {
        return { width, height };
    }

    static unsigned int circle_prog;
    static unsigned int triangle_prog;

    struct BufferObjs {
        unsigned int vao, vbo, ebo;
    };
    static BufferObjs tro, sqo;

    struct VertexData {
        glm::vec3 pos;
        glm::vec2 t_pos;
    };

    Texture default_texture;
    static unsigned int create_shader(std::string vert_shader_file, std::string frag_shader_file) {
        std::vector<uint8_t> vertShaderSrc;
        std::vector<uint8_t> fragShaderSrc;

        vert_shader_file = "shaders/" + vert_shader_file;
        frag_shader_file = "shaders/" + frag_shader_file;

        std::ifstream vertShaderfile(vert_shader_file.c_str(), std::ios::binary | std::ios::ate);
        std::ifstream fragShaderfile(frag_shader_file.c_str(), std::ios::binary | std::ios::ate);

        vertShaderSrc.resize((int)vertShaderfile.tellg() + 1);
        fragShaderSrc.resize((int)fragShaderfile.tellg() + 1);

        vertShaderfile.seekg(std::ios::beg);
        fragShaderfile.seekg(std::ios::beg);

        vertShaderfile.read((char*)vertShaderSrc.data(), vertShaderSrc.size() - 1);
        fragShaderfile.read((char*)fragShaderSrc.data(), vertShaderSrc.size() - 1);

        vertShaderSrc.back() = 0;
        fragShaderSrc.back() = 0;

        vertShaderfile.close();
        fragShaderfile.close();

        char* vertexShaderSource = (char*)vertShaderSrc.data();
        char* fragmentShaderSource = (char*)fragShaderSrc.data();

        // vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            std::cout << "File : " << vert_shader_file << " and " << frag_shader_file << std::endl;
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // link shaders
        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return shaderProgram;
    }

    static BufferObjs create_buffer(std::vector<VertexData> vertices, std::vector<unsigned int > indices) {
        BufferObjs buf;
        glGenVertexArrays(1, &buf.vao);
        glGenBuffers(1, &buf.vbo);
        glGenBuffers(1, &buf.ebo);

        glBindVertexArray(buf.vao);

        glBindBuffer(GL_ARRAY_BUFFER, buf.vbo);
        glBufferData(GL_ARRAY_BUFFER,
            vertices.size() * sizeof(vertices[0]),
            vertices.data(),
            GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(indices[0]),
            indices.data(),
            GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return buf;
    }

    static Texture generateTexture(unsigned char* data, int wid, int hei, int channels) {

        Texture tex;
        tex.width = wid;
        tex.height = hei;
        tex.channels = channels;
        //Create textures
        glGenTextures(1, &tex.tex);
        glBindTexture(GL_TEXTURE_2D, tex.tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        //Map data to texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        return tex;
    }

    bool createTexture(std::string img_file, Texture& tex) {
        //Load image data
        stbi_set_flip_vertically_on_load(true);
        unsigned char* file_data = stbi_load(img_file.c_str(), &tex.width, &tex.height, &tex.channels, 4);
        if (!file_data) {
            stbi_image_free(file_data);
            return false;
        }
        stbi_set_flip_vertically_on_load(false);
        tex = generateTexture(file_data, tex.width, tex.height, tex.channels);

        stbi_image_free(file_data);
        return true;
    }

    static void init_shaders() {
        triangle_prog = create_shader("triangle.vs", "triangle.fs");
        circle_prog = create_shader("circle.vs", "circle.fs");
    }

    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }

    static void init_vertices() {
        std::vector<VertexData> vertices1 = {
            {{  1.f,  1.f, 0.0f} , {1.0f, 1.0f}},  // top right
            {{  1.f, -1.f, 0.0f} , {1.0f, 0.0f}},  // bottom right
            {{ -1.f, -1.f, 0.0f} , {0.0f, 0.0f}},  // bottom left
            {{ -1.f,  1.f, 0.0f} , {0.0f, 1.0f}}   // top left 
        };
        std::vector<unsigned int> indices1 = {  // note that we start from 0!
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
        };

        std::vector<VertexData> vertices2 = {

            {{0.f,2.f     ,0.f},{ 0.5f,1.5f}},
            {{sqrtf(3),-1 ,0.f},{ 1.366025403f,0.0f}},
            {{-sqrtf(3),-1,0.f},{-0.366025403f,0.0f}}
        };

        std::vector<unsigned int> indices2 = {
            0, 1, 2
        };

        sqo = create_buffer(vertices1, indices1);
        tro = create_buffer(vertices2, indices2);
    }
    //Callback std::function objects
    std::function<void(int button, int action, int mods)> mouse_click_callback = [](int, int, int)->void {};
    std::function<void(int button, int action, int mods)> keyboard_callback = [](int, int, int)->void {};
    std::function<void(double delx, double dely)> cursor_move_callback = [](double, double)->void {};
    std::function<void(double xoff, double yoff)> scroll_callback = [](double, double)-> void {};
    //Static callback functions
    static void framebuffer_size_callback(GLFWwindow* window, int new_width, int new_height) {
        width = new_width;
        height = new_height;
        glViewport(0, 0, width, height);
    }
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        mouse_click_callback(button, action, mods);
    }
    static void key_button_callback(GLFWwindow* window, int button, int scancode, int action, int mods) {
        keyboard_callback(button, action, mods);
    }

    static void mouse_pos_move_callback(GLFWwindow* window, double xpos, double ypos) {
        cursor_move_callback(xpos - mouse_pos.x, height - ypos - mouse_pos.y);
        mouse_pos.x = xpos;
        mouse_pos.y = height - ypos;
        
    }
    static void mouse_scroll_callback(GLFWwindow* window, double xoff, double yoff) {
        scroll_callback(xoff, yoff);
    }
    // Other helper functions
    void set_window_size(int width, int height) {
        glfwSetWindowSize(window, width, height);
    }
    void set_window_title(std::string title) {
        glfwSetWindowTitle(window, title.c_str());
    }
    void set_window_icon(std::string icon_file) {
        GLFWimage icon_img;
        int icon_chnl = 4;
        icon_img.pixels = (unsigned char*)stbi_load(
            icon_file.c_str(),
            & icon_img.width,
            & icon_img.height,
            & icon_chnl,
            icon_chnl);

        glfwSetWindowIcon(Context::window, 1, &icon_img);

        stbi_image_free(icon_img.pixels);

    }
    glm::vec2 get_mouse_pos() {
        return mouse_pos;
    }
    void set_mouse_pos(glm::vec2 pos){
        glm::vec2 p2 = { pos.x, height - pos.y };
        glfwSetCursorPos(window, p2.x, p2.y);
    }
    bool is_mouse_button_pressed(int mouse_button) {
        return glfwGetMouseButton(window, mouse_button) == GLFW_PRESS;
    }
    bool is_key_pressed(int scancode){
        return glfwGetKey(window, scancode) == GLFW_PRESS;
    }

    void set_close_window(){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    void init_rendering(glm::vec3 clear_col) {
        glClearColor(clear_col.r, clear_col.g, clear_col.b, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    bool poll_events_and_decide_quit() {
        glfwPollEvents();
        return glfwWindowShouldClose(Context::window);
    }
    void set_fullscreen(bool value){
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        int height = mode->height;
        if (!value) {
            monitor = nullptr;
            height -= 20;
        }
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, height, mode->refreshRate);
    }
    void finish_rendering() {

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(Context::window);
    }

    int init() {
        glfwSetErrorCallback(glfw_error_callback);

        glfwInit();
        

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        const char* glsl_version = "#version 330";


        //Setup callbacks
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetKeyCallback(window, key_button_callback);
        glfwSetScrollCallback(window, mouse_scroll_callback);
        glfwSetCursorPosCallback(window, mouse_pos_move_callback);

        


        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(Context::window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        init_shaders();
        init_vertices();

        //Generate a default texture for non textured rendering
        unsigned char* tmp_dummy_arr = (unsigned char *)malloc(width * height * 4);
        memset(tmp_dummy_arr, 0xff, width * height * 4);
        default_texture = generateTexture(tmp_dummy_arr, width, height, 4);
        free(tmp_dummy_arr);

        return 1;
    }
    void clean() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
    }
    
    //Drawing portions
    void Circle::draw(Texture tex, bool reuse_shader )const {
        if (!reuse_shader)
            glUseProgram(circle_prog);

        //Todo :: remember to set a default shader

        glUniform1i(glGetUniformLocation(circle_prog, "texture0"), 0);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex.tex);

        //Collect locations
        unsigned int transformLoc = glGetUniformLocation(circle_prog, "transform");
        unsigned int colLoc = glGetUniformLocation(circle_prog, "colors");
        glm::vec2 center = this->center;
        //Normalize everything
        center.x = -1.f + center.x * (2.f / width);
        center.y = -1.f + center.y * (2.f / height);
        glm::vec2 scale1;
        scale1.x = radius * (2.f / width);
        scale1.y = radius * (2.f / height);

        //Form matrices and store
        glm::mat3 off_mat = { 1.f,0.f,0.f,0.f,1.f,0.f,center.x,center.y,1.f };
        glm::mat3 rot_mat = { cos(rotate), sin(rotate), 0.f, -sin(rotate), cos(rotate), 0.f, 0.f,0.f,1.f };
        glm::mat3 scale0_mat = { scale.x, 0.f,0.f,0.f,scale.y,0.f,0.f,0.f,1.f };
        glm::mat3 scale1_mat = { scale1.x, 0.f,0.f,0.f,scale1.y,0.f,0.f,0.f,1.f };
        glm::mat3 final_mat = off_mat * scale1_mat * rot_mat * scale0_mat;
        glUniformMatrix3fv(transformLoc, 1, GL_FALSE, glm::value_ptr(final_mat));

        //Store color and draw
        glUniform3fv(colLoc, 1, glm::value_ptr(color));

        glBindVertexArray(Context::tro.vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Context::tro.ebo);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

    }
    
    void Triangle::draw(Texture tex, bool reuse_shader )const {
            if (!reuse_shader)
                glUseProgram(triangle_prog);

            //Todo :: remember to set a default shader

            glUniform1i(glGetUniformLocation(triangle_prog, "texture0"), 0);

            // bind textures on corresponding texture units
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex.tex);

            //Collect locations
            unsigned int transformLoc = glGetUniformLocation(triangle_prog, "transform");
            unsigned int colLoc = glGetUniformLocation(triangle_prog, "colors");

            glm::vec2 pos1 = this->pos1;
            glm::vec2 pos2 = this->pos2;
            glm::vec2 pos3 = this->pos3;

            //Normalize everything
            pos1.x = -1.f + pos1.x * (2.f / width);
            pos1.y = -1.f + pos1.y * (2.f / height);

            pos2.x = -1.f + pos2.x * (2.f / width);
            pos2.y = -1.f + pos2.y * (2.f / height);

            pos3.x = -1.f + pos3.x * (2.f / width);
            pos3.y = -1.f + pos3.y * (2.f / height);

            glm::vec2 src0{ 0.0f,2.f },
                src1{ glm::sqrt(3) ,-1.f },
                src2{ -glm::sqrt(3) ,-1.f };

            // For each positions,
            // [ a00 a01 a02 ] [x] = [x']
            // [ a10 a11 a12 ] [y] = [y']
            // [ 000 000 001 ] [1] = [01]
            //

            //This below just works
            //Basic principle being solving 2 sets of linear equations 
            //Never going to touch it again now
            glm::mat3 matA;
            glm::mat3 matB;
            matA = { {src0,1.f},{src1,1.f},{src2,1.f} };
            matB = { {pos1,1.f},{pos2,1.f},{pos3,1.f} };
            glm::mat3 inv = glm::inverse(matA);

            glm::mat3 matBt = glm::transpose(matB);
            glm::mat3 invt = glm::transpose(inv);
            glm::vec3 row1 = invt * matBt[0];
            glm::vec3 row2 = invt * matBt[1];


            glm::mat3 res_mat = { row1, row2, {0.f,0.f,1.f} };
            res_mat = glm::transpose(res_mat);

            glUniformMatrix3fv(transformLoc, 1, GL_FALSE, glm::value_ptr(res_mat));

            //Store color and draw
            glUniform3fv(colLoc, 1, glm::value_ptr(color));

            glBindVertexArray(Context::tro.vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Context::tro.ebo);
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        }
    
    void Rectangle::draw(Texture tex, bool reuse_shader )const {
            if (!reuse_shader)
                glUseProgram(triangle_prog);

            //Todo :: remember to set a default shader

            glUniform1i(glGetUniformLocation(triangle_prog, "texture0"), 0);

            // bind textures on corresponding texture units
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex.tex);

            //Collect locations
            unsigned int transformLoc = glGetUniformLocation(triangle_prog, "transform");
            unsigned int colLoc = glGetUniformLocation(triangle_prog, "colors");
            //
            ////Normalize everything
            glm::vec2 center = this->center;
            center.x = -1.f + center.x * (2.f / width);
            center.y = -1.f + center.y * (2.f / height);

            glm::mat3 off_mat = { 1.f,0.f,0.f,0.f,1.f,0.f,center.x,center.y,1.f };
            glm::mat3 rot_mat = { cos(rotate), sin(rotate), 0.f, -sin(rotate), cos(rotate), 0.f, 0.f,0.f,1.f };
            glm::mat3 scale0_mat = { size.x, 0.f,0.f,0.f,size.y,0.f,0.f,0.f,1.f };
            glm::mat3 scale1_mat = { scale.x, 0.f,0.f,0.f,scale.y,0.f,0.f,0.f,1.f };

            glm::mat3 scale_scr_mat = { 1.f / width, 0.f,0.f,0.f,1.f / height,0.f,0.f,0.f,1.f };


            glm::mat3 final_mat = off_mat * scale_scr_mat * scale1_mat * rot_mat * scale0_mat;

            glUniformMatrix3fv(transformLoc, 1, GL_FALSE, glm::value_ptr(final_mat));

            //Store color and draw
            glUniform3fv(colLoc, 1, glm::value_ptr(color));

            glBindVertexArray(Context::sqo.vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Context::sqo.ebo);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        }
    
    void Line::draw(Texture tex, bool reuse_shader ) {
            //Form matrices and store

            float rotate = atan2f(pos2.y - pos1.y, pos2.x - pos1.x);
            float length = glm::length(pos2 - pos1);
            glm::vec2 offset = 0.5f * (pos2 + pos1);

            Rectangle{
                .center = offset,
                .size = {length, line_width},
                .color = color,
                .scale = {1.f,1.f},
                .rotate = rotate
            }.draw(tex, reuse_shader);

        }


    
};

int opengl_demo(){

    Context::init();


    Context::Texture baba;
    Context::createTexture("baba-1.png", baba);

    double angle = 3.14 / 360;




    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while (!Context::poll_events_and_decide_quit())
    {
        

        Context::init_rendering(Color::silver);
        Context::Circle{
    .center = Context::get_real_dim() * 0.5f,
    .radius = Context::height * 0.5f,
    .color = {clear_color.x,clear_color.y,clear_color.z},
    .scale = {0.5f,1.f},
    .rotate = (float)angle
        }.draw(baba);

        angle += 3.14 / 500;

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering

        Context::finish_rendering();
    }

    Context::clean();
    return 0;
}

