//
// Created by giovanni on 12/06/22.
//

#ifndef BREAKOUT_TEXTRENDERER_H
#define BREAKOUT_TEXTRENDERER_H


#include <glm/glm.hpp>
#include <map>
#include "shader.h"
#include <iostream>
#include "resource_manager.h"

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};


class TextRenderer{
public:
    std::map<char, Character> Characters;
    Shader TextShader;
    TextRenderer(unsigned int width, unsigned int height);
    void Load(std::string font, unsigned int fontSize);
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0));
private:
    unsigned int VAO, VBO;
};

#endif //BREAKOUT_TEXTRENDERER_H
