#include "Framebuffer.h"


Framebuffer::Framebuffer()
{
}


void Framebuffer::generate()
{
    glCreateFramebuffers(1, &ID);
    colorAttachment.reserve(16);
}

void Framebuffer::initRenderBuffer(std::string name, int width, int height, GLenum colorMinFilter, GLenum colorMagFilter, GLenum GPUformat, GLenum CPUformat)
{
    colorAttachment.emplace_back(GPUformat, CPUformat, width, height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, colorMinFilter, colorMagFilter, false, false);

    glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0, colorAttachment[0].id, 0);

    glCreateRenderbuffers(1, &depthAttachment);
    glNamedRenderbufferStorage(depthAttachment, GL_DEPTH24_STENCIL8, width, height);
    glNamedFramebufferRenderbuffer(ID, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthAttachment);

    if (glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer is not complete of name: " << name << "!" << std::endl;
    }
}

void Framebuffer::initTexture(std::string name, int width, int height, GLenum colorMinFilter, GLenum colorMagFilter, GLenum depthMinFilter, GLenum depthMagFilter, GLenum GPUformat, GLenum CPUformat)
{
    colorAttachment.emplace_back(GPUformat, CPUformat, width, height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, colorMinFilter, colorMagFilter, false, false);

    glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0, colorAttachment[0].id, 0);

    glCreateTextures(GL_TEXTURE_2D, 1, &depthAttachment);
    glTextureStorage2D(depthAttachment, 1, GL_DEPTH24_STENCIL8, width, height);
    glTextureParameteri(depthAttachment, GL_TEXTURE_MIN_FILTER, depthMinFilter);
    glTextureParameteri(depthAttachment, GL_TEXTURE_MAG_FILTER, depthMagFilter);
    glTextureParameteri(depthAttachment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(depthAttachment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(ID, GL_DEPTH_STENCIL_ATTACHMENT, depthAttachment, 0);

    if (glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer is not complete of name: " << name << "!" << std::endl;
    }
}

void Framebuffer::addColorAttachment(GLenum GPUformat,GLenum CPUformat,int width,int height,GLenum minFilter,GLenum magFilter)
{
    colorAttachment.emplace_back(GPUformat, CPUformat, width, height, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, minFilter, magFilter, false, false);
    glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0 + colorAttachment.size() - 1,colorAttachment.back().id, 0);
}

void Framebuffer::use()
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
}
