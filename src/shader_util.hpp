#pragma once
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <vector>

int compileShader(GLuint shader);
int linkProgram(GLuint program);
GLuint createShaderFromBinary(const char *path, GLenum type);
GLuint createShaderFromSource(const char *path, GLenum type);

inline int compileShader(GLuint shader)
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

inline int linkProgram(GLuint program)
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

inline GLuint createShaderFromBinary(const char *path, GLenum type)
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

inline GLuint createShaderFromSource(const char* path, GLenum type) {
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