#version 460 core


struct GlyphInfo {
    vec4 atlasBounds;
    vec4 planeBounds; 
    vec2 origin;
    vec2 padding;
};

layout(std430,binding = 5) readonly buffer textBuffer
{
    GlyphInfo glyphs[];   
};

uniform vec2 u_textureSize;
uniform float size;
uniform mat4 u_ortho;

out vec2 texCoord;

vec2 uvs[6] = vec2[6]
(
    vec2(0,0),vec2(1,0),vec2(1,1),
    vec2(1,1),vec2(0,1),vec2(0,0)
);

void main()
{
    int charIndex = gl_VertexID / 6;
    int vertIndex = gl_VertexID % 6;

    vec2 corner = uvs[vertIndex];
    GlyphInfo g = glyphs[charIndex];

    float posX = mix(g.planeBounds.x,g.planeBounds.z,corner.x) * size + g.origin.x;
    float posY = mix(g.planeBounds.y,g.planeBounds.w,corner.y) * size + g.origin.y;

    float u = mix(g.atlasBounds.x,g.atlasBounds.z,corner.x) / u_textureSize.x;
    float v = mix(g.atlasBounds.y,g.atlasBounds.w,corner.y) / u_textureSize.y;

    texCoord = vec2(u,v);

    gl_Position = u_ortho * vec4(posX,posY,0.0f,1.0f);

}