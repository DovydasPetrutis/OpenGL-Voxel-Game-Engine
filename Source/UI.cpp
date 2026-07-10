#include "UI.h"

void UserInterface::drawText(const std::string& text, int x, int y, float size, glm::vec3 color, Shader& drawProgram, std::unordered_map<int, glyphData>& font, GLuint buffer, GLuint textTexture)
{
    drawProgram.use();
    drawProgram.uniform3f("u_color", color);
    drawProgram.uniform1f("size", size);
    glm::vec2 origin = glm::vec2(x, y);
    std::vector<glyphVertex> characters;
    int length = text.length();
    characters.reserve(length);
    for (int i = 0;i < length;i++)
    {
        glyphData g = font[(int)text[i]];
        glyphVertex v;
        v.atlasBounds = g.atlasBounds;
        v.planeBounds = g.planeBounds;
        v.origin = origin;
        characters.push_back(v);
        origin.x += g.advance * size;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glyphVertex) * length, characters.data());
    glDrawArrays(GL_TRIANGLES, 0, 6 * length);
}

void UserInterface::loadGlyphs(std::unordered_map<int, glyphData>& charDatas, const std::string& fontMetadataPath)
{
    std::ifstream f(fontMetadataPath);
    nlohmann::json data = nlohmann::json::parse(f);
    f.close();
    for (auto& glyph : data["glyphs"])
    {
        glyphData gData;
        if (glyph.contains("atlasBounds"))
        {
            auto atlasBounds = glyph["atlasBounds"];

            gData.atlasBounds.x = atlasBounds["left"];
            gData.atlasBounds.y = atlasBounds["bottom"];
            gData.atlasBounds.z = atlasBounds["right"];
            gData.atlasBounds.w = atlasBounds["top"];
        }
        else
        {
            gData.atlasBounds = glm::vec4(0.0f);
        }
        if (glyph.contains("planeBounds"))
        {
            auto planeBounds = glyph["planeBounds"];

            gData.planeBounds.x = planeBounds["left"];
            gData.planeBounds.y = planeBounds["bottom"];
            gData.planeBounds.z = planeBounds["right"];
            gData.planeBounds.w = planeBounds["top"];
        }
        else
        {
            gData.planeBounds = glm::vec4(0.0f);
        }

        gData.advance = glyph["advance"];

        charDatas[glyph["unicode"]] = gData;
    }
}
