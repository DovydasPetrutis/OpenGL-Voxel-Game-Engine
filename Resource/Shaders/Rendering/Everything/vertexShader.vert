#version 460 core
#extension GL_ARB_shader_draw_parameters : enable

out vec3 aTexCoords;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 prevView;
uniform mat4 prevModel;
uniform mat4 prevProj;
uniform mat4 noJitterProj;
uniform bool TAA;

out vec4 currentClipPos;
out vec4 previousClipPos;

struct ChunkInput {
    vec4 worldPos;
    uint facesCount;
    uint faceOffset;
    uint pad[2];
};

layout(std430,binding = 3) readonly buffer face
{
    uint packedFace[];
};

layout(std430,binding = 0)readonly buffer inputBuffer
{
    ChunkInput chunks[];
};

layout(std430,binding = 4) buffer mappingBuffer
{
    uint visibleChunksId[];
};

vec4 corners[6] = vec4[](
    vec4(-0.5,-0.5,0.0f,0.0f), 
    vec4( 0.5,-0.5,1.0f,0.0f), 
    vec4(-0.5, 0.5,0.0f,1.0f),
    vec4( 0.5,-0.5,1.0f,0.0f), 
    vec4( 0.5, 0.5,1.0f,1.0f), 
    vec4(-0.5, 0.5,0.0f,1.0f)
);

void main()
{
    vec3 localPos;
    uint chunkID = visibleChunksId[gl_DrawID];
    vec3 chunkWorldPos = chunks[chunkID].worldPos.xyz;
    uint faceData = packedFace[gl_InstanceID + gl_BaseInstance];
    uint lx = (faceData & 0xFu);
    uint ly = (faceData >> 4) & 0xFu;
    uint lz = (faceData >> 8) & 0xFu;
    uint faceId = (faceData >> 12) & 0x7u;
    vec4 corner = corners[gl_VertexID % 6];
    if(faceId == 0) localPos = vec3(-corner.x,corner.y,-0.5f); // Front
    else if(faceId == 1) localPos = vec3(corner.x,corner.y,0.5f); // Back
    else if(faceId == 2) localPos = vec3(0.5f,corner.y,-corner.x); // Right
    else if(faceId == 3) localPos = vec3(-0.5f,corner.y,corner.x); // Left
    else if(faceId == 4) localPos = vec3(-corner.x,0.5f,corner.y); // Top
    else if(faceId == 5) localPos = vec3(corner.x,-0.5f,corner.y); // Bottom
    vec3 finalPos =  localPos + vec3(lx,ly,lz) + chunkWorldPos; // Chunks
    uint layer = (faceData >> 15);
    aTexCoords = vec3(corner.z,corner.w,float(layer));
    if(TAA)
    {
        currentClipPos = noJitterProj * view * model * vec4(finalPos,1.0);
        previousClipPos = prevProj * prevView * prevModel * vec4(finalPos,1.0);
    }
    gl_Position = proj * view * model * vec4(finalPos,1.0);
    
}