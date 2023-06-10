#define SHADER_BIN

#include "font_renderer.hpp"
#include "shader_util.hpp"

#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

static glm::mat4 textProjection;
static GLuint textRenderProgram, vbo, vao;

int InitFontRenderer()
{
    FT_Library ft;
    if(FT_Init_FreeType(&ft)) {
        std::cout << "Could not initialize freetype library" << std::endl;
        return -1;
    }

    FT_Face face;
    if(FT_New_Face(ft, "resources/arial.ttf", 0, &face)) {
        std::cout << "Failed to load font" << std::endl;
        return -1;
    }
    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for(unsigned char c = 0; c < 128; c++) {
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "Failed to load Glyph '" << c << "'" << std::endl;
            continue;
        }
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (unsigned int)face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    textProjection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

    textRenderProgram = glCreateProgram();
#ifdef SHADER_BIN
    GLuint vertex = createShaderFromBinary("resources/render_font.vert.spv", GL_VERTEX_SHADER);
    GLuint fragment = createShaderFromBinary("resources/render_font.frag.spv", GL_FRAGMENT_SHADER);
#else
    GLuint vertex   = createShaderFromSource("resources/render_font.vert", GL_VERTEX_SHADER);
    GLuint fragment = createShaderFromSource("resources/render_font.vert", GL_FRAGMENT_SHADER);
#endif

    glAttachShader(textRenderProgram, vertex);
    glAttachShader(textRenderProgram, fragment);

    if(!linkProgram(textRenderProgram)) {
        std::cout << "Error linking program" << std::endl;
        return -1;
    }

    glDetachShader(textRenderProgram, vertex);
    glDetachShader(textRenderProgram, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(textRenderProgram);
    glUniformMatrix4fv(glGetUniformLocation(textRenderProgram, "projection"), 1, false, glm::value_ptr(textProjection));

    return 0;
}

void CleanupFontRenderer()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(textRenderProgram);
}

void RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
    glUseProgram(textRenderProgram);
    glUniform3fv(glGetUniformLocation(textRenderProgram, "textColor"), 1, glm::value_ptr(color));
    glBindVertexArray(vao);

    std::string::const_iterator c;
    for(c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
