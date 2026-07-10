#pragma once
#include "Shader.h"
#include "Settings.h"
#include "Camera.h"
#include "Utils.h"
#include "Buffers.h"
#include "World.h"
#include "UI.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "TextureArray.h"
#include "Player.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>





struct DrawArraysIndirectCommand {
    uint32_t count;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t baseInstance;
};

struct CallbackStruct
{
    ComputeShader* cullShader;
    Shader* projShader;
    Shader* screenShader;
    Shader* taaShader;
    Camera* camera;
    int xLast;
    int yLast;
    bool firstMouse;
    void set(ComputeShader* cullShader, Shader* projShader, Shader* screenShader, Shader* taaShader, Camera* camera, int xLast, int yLast, bool firstMouse)
    {
        this->cullShader = cullShader;
        this->projShader = projShader;
        this->screenShader = screenShader;
        this->taaShader = taaShader;
        this->camera = camera;
        this->xLast = xLast;
        this->yLast = yLast;
        this->firstMouse = firstMouse;
    }
};

class Render
{
public:
    GLFWwindow* window;

    CallbackStruct context;

    Camera camera;

    VAO VAO;

    VBO VBOoutline;
    VBO prevVBOoutline;

    SSBO VBOfaces;
    SSBO cullInputBuffer;
    SSBO cullOutputBuffer;
    SSBO cullCountBuffer;
    SSBO mappingBuffer;

    SSBO textBuffer;
    
    Texture previousVelocityTex;
    Framebuffer mainFBO;
    Framebuffer history1FBO;
    Framebuffer history2FBO;

    TextureArray textures;
    Texture characters;
    std::unordered_map<int, glyphData> minecraftCharDatas;

    Shader program;
    Shader hudProgram;
    Shader textProgram;
    Shader screenProgram;
    Shader TAAProgram;
    Shader outlineProgram;
    ComputeShader cullProgram;

    // Variables used in rendering
    bool firstMouse;
    int xLast;
    int yLast;
    int taaFrames;
    uint32_t cullCountNumber;
    float currentFrame;
    float lastFrame;
    float deltaTime;
    bool oddframe;
    int nbFrames;
    int lastTime;
    int lastFps;

    glm::mat4 model;
    glm::mat4 proj;
    glm::mat4 noJitterProj;
    glm::mat4 prevProj = noJitterProj;
    glm::mat4 view;
    glm::mat4 prevView;

    glm::vec2 jitter;
    glm::vec2 offset;

    std::vector<glm::mat4> outlineMatrices;
    std::vector<glm::mat4> prevOutlineMatrices;
    glm::mat4 currentOutlineModel;

    GLenum drawMainBuffers[2];
    GLenum drawNormalBuffers[1];

    Render();

    void initWindow();
    void initBuffers(World &world);
    void initTextures();
    void initShaders();
    void initRenderVariables(World& world, Player& player);
    void init(World& world, Player& player);

    void process_input(GLFWwindow* window, Player& player);
    void renderSetSettingsAndVariables();
    void cullingAndProgramUse(World& world);
    void TAA1();
    void setMVPAndRender(World& world);
    void playerLookAtBlockRender(World& world,Player& player);
    void TAA2();
    void drawToQuad();
    void renderHUD(Player& player);
    void renderMiscEnd();

    void render(World& world, Player& player);
};



