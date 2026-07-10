#pragma once
#include <string>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include "Shader.h"

struct glyphData
{
    float advance;
    // left,bottom,right,top
    glm::vec4 planeBounds;
    glm::vec4 atlasBounds;
};

struct glyphVertex
{
    glm::vec4 atlasBounds;
    glm::vec4 planeBounds;
    glm::vec2 origin;
    glm::vec2 padding;
};

class UserInterface
{
public:
	static void drawText(const std::string& text, int x, int y, float size, glm::vec3 color, Shader& drawProgram, std::unordered_map<int, glyphData>& font, GLuint buffer, GLuint textTexture);
    static void loadGlyphs(std::unordered_map<int, glyphData>& charDatas, const std::string& fontMetadataPath);

};
