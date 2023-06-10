#pragma once

#include <glm/glm.hpp>
#include <map>
#include <string>

struct Character {
    unsigned int TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    unsigned int Advance;
};

static std::map<char, Character> Characters;

int InitFontRenderer();
void CleanupFontRenderer();
void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);
