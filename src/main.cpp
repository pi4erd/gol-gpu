#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Uncomment to use shader binary
#define SHADER_BIN

GLFWwindow* window;

GLuint vbo, ebo, vao, fbo, fbt, blitprg, golcomp;

int Width = 800, Height = 600;

const float vertices[] = {
    -1, -1, 0, 0, 0,
    1, -1, 0, 1, 0,
    1, 1, 0, 1, 1,
    -1, 1, 0, 0, 1
};

const GLuint indices[] = {
    0, 1, 2,
    0, 2, 3
};

double _time = 0;

double delay = 0;
int frames = 0;

/// @brief Compiles shader and returns error if unsuccessful
/// @param shader Shader id
/// @return Is successfully compiled
int compileShader(GLuint shader);
/// @brief Links program and returns error if unsuccessful
/// @param program Program id
/// @return Is successfully linked
int linkProgram(GLuint program);
GLuint createShaderFromBinary(const char* path, GLenum type);
void init();
void render(double delta);
void end();
void resize(GLFWwindow* window, int width, int height);

int main() {
    glfwInit();

    glfwWindowHint(GLFW_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(Width, Height, "Game Of Life", nullptr, nullptr);
    if(!window) {
        std::cerr << "Error creating window!" << std::endl;
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetWindowSizeCallback(window, resize);

    glewInit();

    glfwSetTime(0);

    init();
    while(!glfwWindowShouldClose(window)) {
        double delta = glfwGetTime() - _time;
        render(delta);
        _time = glfwGetTime();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    end();
    return 0;
}

int compileShader(GLuint shader)
{
    glCompileShader(shader);

    int success;
    char infoLog[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Error while compiling shader (" << shader << ")\n" << infoLog << std::endl;
        return 0;
    }

    return 1;
}

int linkProgram(GLuint program)
{
    glLinkProgram(program);

    int success;
    char infoLog[512];

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Error while compiling shader (" << program << ")\n" << infoLog << std::endl;
        return 0;
    }

    return 1;
}

GLuint createShaderFromBinary(const char *path, GLenum type)
{
    using namespace std;

    ifstream binfile(path, ios::binary);

    vector<unsigned char> buffer(istreambuf_iterator<char>(binfile), {});

    GLuint shader = glCreateShader(type);
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, buffer.data(), buffer.size());
    glSpecializeShader(shader, "main", 0, 0, 0);

    int success;

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Error compiling binary shader:\n" << infoLog << std::endl;
        return -1;
    }

    /*if(!compileShader(shader)) {
        std::cerr << "Compilation failed" << std::endl;
        return -1;
    }*/

    return shader;
}

GLuint createShaderFromSource(const char* path, GLenum type) {
    using namespace std;

    ifstream infile(path);

    string buffer;
    string line;

    while(!infile.eof()) {
        getline(infile, line);
        buffer += line + '\n';
    }

    const char* source = buffer.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);

    if(!compileShader(shader)) {
        std::cerr << "Compilation failed" << std::endl;
        return -1;
    }

    return shader;
}

void init()
{
    glClearColor(0, 0, 0, 1);

    blitprg = glCreateProgram();
    golcomp = glCreateProgram();
    #ifdef SHADER_BIN
    GLuint vx = createShaderFromBinary("resources/vertex.vert.spv", GL_VERTEX_SHADER);
    GLuint fg = createShaderFromBinary("resources/fragment.frag.spv", GL_FRAGMENT_SHADER);
    GLuint cmpsh = createShaderFromBinary("resources/compute.comp.spv", GL_COMPUTE_SHADER);
    #else
    GLuint vx = createShaderFromSource("resources/vertex.vert", GL_VERTEX_SHADER);
    GLuint fg = createShaderFromSource("resources/fragment.frag", GL_FRAGMENT_SHADER);
    GLuint cmpsh = createShaderFromSource("resources/compute.comp", GL_COMPUTE_SHADER);
    #endif

    glAttachShader(blitprg, vx);
    glAttachShader(blitprg, fg);
    glAttachShader(golcomp, cmpsh);

    if(!linkProgram(blitprg)) {
        std::cerr << "Program link failed" << std::endl;
    }
    if(!linkProgram(golcomp)) {
        std::cerr << "Program link failed" << std::endl;
    }

    glDetachShader(blitprg, vx);
    glDetachShader(blitprg, fg);
    glDetachShader(golcomp, cmpsh);
    glDeleteShader(vx);
    glDeleteShader(fg);
    glDeleteShader(cmpsh);

    glUseProgram(golcomp);
    glDispatchCompute(Width, Height, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glCreateTextures(GL_TEXTURE_2D, 1, &fbt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureStorage2D(fbt, 1, GL_RGBA32F, Width, Height);
    glBindImageTexture(0, fbt, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void render(double delta) {
    glUseProgram(golcomp);
    glDispatchCompute(Width, Height, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glUseProgram(blitprg);
    glBindTextureUnit(0, fbt);
    glUniform1i(glGetUniformLocation(blitprg, "mainTexture"), 0);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

    delay -= delta;

    if(delay <= 0) {
        char newTitle[64];
        sprintf(newTitle, "Game Of Life | %d fps", frames);
        glfwSetWindowTitle(window, newTitle);
        delay = 1;
        frames = 0;
    }
    frames++;
}
void end() {
    glDeleteProgram(blitprg);
    glDeleteProgram(golcomp);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(1, &fbt);
    glDeleteFramebuffers(1, &fbo);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void resize(GLFWwindow *window, int width, int height)
{
    Width = width;
    Height = height;
}
