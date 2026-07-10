#version 460 core

uniform float thickness;
uniform float scale;

vec3 rectangle[36] = vec3[](
    vec3(-0.5+scale          ,-0.5-thickness+scale,          -0.5-thickness+scale),
    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          -0.5-thickness+scale),
    vec3(-0.5+scale        ,-0.5+thickness-thickness+scale,-0.5-thickness+scale),

    vec3(-0.5 +scale         ,-0.5+thickness-thickness+scale,-0.5-thickness+scale),
    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          -0.5-thickness+scale),
    vec3(-0.5-thickness+scale,-0.5+thickness-thickness+scale,-0.5-thickness+scale),



    vec3(-0.5+scale         ,-0.5-thickness+scale,          -0.5-thickness+scale),
    vec3(-0.5+scale          ,-0.5-thickness+scale,           0.5+thickness-scale),
    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          -0.5-thickness+scale),

    vec3(-0.5+scale         ,-0.5+thickness-thickness+scale,-0.5-thickness+scale),
    vec3(-0.5-thickness+scale,-0.5+thickness-thickness+scale, 0.5+thickness-scale),
    vec3(-0.5+scale          ,-0.5+thickness-thickness+scale, 0.5+thickness-scale),
    
    vec3(-0.5+scale          ,-0.5+thickness-thickness+scale,-0.5-thickness+scale),
    vec3(-0.5-thickness+scale,-0.5+thickness-thickness+scale,-0.5-thickness+scale),
    vec3(-0.5-thickness+scale,-0.5+thickness-thickness+scale, 0.5+thickness-scale),

    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          -0.5-thickness+scale),
    vec3(-0.5+scale          ,-0.5-thickness+scale,          0.5+thickness-scale),
    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          0.5+thickness-scale),

    vec3(-0.5+scale          ,-0.5-thickness+scale,          -0.5-thickness+scale),
    vec3(-0.5+scale         ,-0.5+thickness-thickness+scale,0.5+thickness-scale),
    vec3(-0.5+scale         ,-0.5-thickness+scale,          0.5+thickness-scale),
    
    vec3(-0.5+scale          ,-0.5+thickness-thickness+scale,-0.5-thickness+scale),
    vec3(-0.5+scale          ,-0.5+thickness-thickness+scale,0.5+thickness-scale),
    vec3(-0.5+scale          ,-0.5-thickness+scale,          -0.5-thickness+scale),

    vec3(-0.5-thickness+scale,-0.5+thickness-thickness+scale,-0.5-thickness+scale),
    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          -0.5-thickness+scale),
    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          0.5+thickness-scale),

    vec3(-0.5-thickness+scale,-0.5+thickness-thickness+scale,-0.5-thickness+scale),
    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          0.5+thickness-scale),
    vec3(-0.5-thickness+scale,-0.5+thickness-thickness+scale,0.5+thickness-scale),
    


    vec3(-0.5+scale          ,-0.5-thickness+scale,          0.5+thickness-scale),
    vec3(-0.5+scale        ,-0.5+thickness-thickness+scale,0.5+thickness-scale),
    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          0.5+thickness-scale),
    

    vec3(-0.5-thickness+scale,-0.5-thickness+scale,          0.5+thickness-scale),
    vec3(-0.5+scale          ,-0.5+thickness-thickness+scale,0.5+thickness-scale),
    vec3(-0.5-thickness+scale,-0.5+thickness-thickness+scale,0.5+thickness-scale)
);

layout(location = 0) in mat4 instanceModel;    
layout(location = 4) in mat4 instancePrevModel;
uniform mat4 view;
uniform mat4 prevView;
uniform mat4 proj;        // jittered
uniform mat4 noJitterProj;
uniform mat4 prevProj;    // last frame's noJitterProj
uniform bool drawCorners;

out vec4 currentClipPos;
out vec4 previousClipPos;
out vec3 worldPos;

void main()
{
    vec4 pos = vec4(rectangle[gl_VertexID],1.0);
    currentClipPos  = noJitterProj * view * instanceModel * pos;
    previousClipPos = prevProj     * prevView * instancePrevModel * pos;
    gl_Position     = proj         * view     * instanceModel * pos;
    worldPos = vec3(instanceModel * pos); 
}