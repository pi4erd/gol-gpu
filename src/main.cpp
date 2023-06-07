#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Uncomment to use shader binary
#define SHADER_BIN

GLFWwindow* window;

GLuint vbo, ebo, vao, fbo, fbt, fbt2, blitprg, golcomp, initprg, copyprg;

int Width = 1920, Height = 1080;

const int TextureWidth = 1024, TextureHeight = 1024;

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
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void resize(GLFWwindow* window, int width, int height);

int main() {
    glfwInit();

    glfwWindowHint(GLFW_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(Width, Height, "Game Of Life", nullptr, nullptr);
    if(!window) {
        std::cerr << "Error creating window!" << std::endl;
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, resize);
    glfwSetScrollCallback(window, scrollCallback);

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

GLuint createTextureFromPath(const char* path, int unit) {
    GLuint result;
    glCreateTextures(GL_TEXTURE_2D, 1, &result);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if(!data) {
        std::cout << "Failed to load texture!" << std::endl;
        return -1;
    }

    glBindTexture(GL_TEXTURE_2D, result);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    glBindTextureUnit(unit, result);

    return result;
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
    initprg = glCreateProgram();
    copyprg = glCreateProgram();
#ifdef SHADER_BIN
    GLuint vx = createShaderFromBinary("resources/vertex.vert.spv", GL_VERTEX_SHADER);
    GLuint fg = createShaderFromBinary("resources/fragment.frag.spv", GL_FRAGMENT_SHADER);
    GLuint cmpsh = createShaderFromBinary("resources/compute.comp.spv", GL_COMPUTE_SHADER);
    GLuint initsh = createShaderFromBinary("resources/initsh.comp.spv", GL_COMPUTE_SHADER);
    GLuint copysh = createShaderFromBinary("resources/copyprg.comp.spv", GL_COMPUTE_SHADER);
#else
    GLuint vx = createShaderFromSource("resources/vertex.vert", GL_VERTEX_SHADER);
    GLuint fg = createShaderFromSource("resources/fragment.frag", GL_FRAGMENT_SHADER);
    GLuint cmpsh = createShaderFromSource("resources/compute.comp", GL_COMPUTE_SHADER);
    GLuint initsh = createShaderFromSource("resources/initsh.comp", GL_COMPUTE_SHADER);
    GLuint copysh = createShaderFromSource("resources/copyprg.comp", GL_COMPUTE_SHADER);
#endif

    glAttachShader(blitprg, vx);
    glAttachShader(blitprg, fg);
    glAttachShader(golcomp, cmpsh);
    glAttachShader(initprg, initsh);
    glAttachShader(copyprg, copysh);

    if(!linkProgram(blitprg)) {
        std::cerr << "Program link failed" << std::endl;
    }
    if(!linkProgram(golcomp)) {
        std::cerr << "Program link failed" << std::endl;
    }
    if(!linkProgram(initprg)) {
        std::cerr << "Program link failed" << std::endl;
    }
    if(!linkProgram(copyprg)) {
        std::cerr << "Program link failed" << std::endl;
    }

    glDetachShader(blitprg, vx);
    glDetachShader(blitprg, fg);
    glDetachShader(golcomp, cmpsh);
    glDetachShader(initprg, initsh);
    glDetachShader(copyprg, copysh);
    glDeleteShader(vx);
    glDeleteShader(fg);
    glDeleteShader(cmpsh);
    glDeleteShader(initsh);
    glDeleteShader(copysh);

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

    glCreateTextures(GL_TEXTURE_2D, 1, &fbt);
    glBindTexture(GL_TEXTURE_2D, fbt);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureStorage2D(fbt, 1, GL_RGBA32F, TextureWidth, TextureHeight);
    glBindImageTexture(0, fbt, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glCreateTextures(GL_TEXTURE_2D, 1, &fbt2);
    glBindTexture(GL_TEXTURE_2D, fbt2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureStorage2D(fbt2, 1, GL_RGBA32F, TextureWidth, TextureHeight);
    glBindImageTexture(1, fbt2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    GLuint inittexture = createTextureFromPath("resources/init.png", 2);
    glBindImageTexture(2, inittexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    glUseProgram(initprg);
    glDispatchCompute(TextureWidth, TextureHeight, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glDeleteTextures(1, &inittexture);
}

float zoom = 1;

float posx = 0, posy = 0;

void render(double delta) {
    glUseProgram(golcomp);
    glDispatchCompute(TextureWidth, TextureHeight, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    glUseProgram(blitprg);
    glBindTextureUnit(0, fbt);
    glBindTextureUnit(1, fbt2);
    glUniform1f(glGetUniformLocation(blitprg, "zoom"), zoom);
    glUniform1f(glGetUniformLocation(blitprg, "aspectRatio"), Width / (float)Height);
    glUniform1f(glGetUniformLocation(blitprg, "posx"), posx);
    glUniform1f(glGetUniformLocation(blitprg, "posy"), posy);
    glUniform1i(glGetUniformLocation(blitprg, "mainTexture"), 0);
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

    glUseProgram(copyprg);
    glDispatchCompute(TextureWidth, TextureHeight, 1);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    delay -= delta;

    if(glfwGetKey(window, GLFW_KEY_D)) {
        posx += delta / zoom;
    }
    if(glfwGetKey(window, GLFW_KEY_A)) {
        posx -= delta / zoom;
    }
    if(glfwGetKey(window, GLFW_KEY_W)) {
        posy += delta / zoom;
    }
    if(glfwGetKey(window, GLFW_KEY_S)) {
        posy -= delta / zoom;
    }
    
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
    glDeleteProgram(initprg);
    glDeleteProgram(copyprg);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(1, &fbt);
    glDeleteTextures(1, &fbt2);
    glDeleteFramebuffers(1, &fbo);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    zoom *= (1 + yoffset * 0.1);
}

void resize(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    Width = width;
    Height = height;
}
