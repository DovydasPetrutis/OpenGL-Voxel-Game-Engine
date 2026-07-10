#version 460 core

void main()
{
    vec2 corners[12] = vec2[](
        // Horizontal Quad: 
        // X goes from -(width + thickness) to +(width + thickness)
        // Y is just the thickness
        vec2(-0.017666f, -0.002666f),
        vec2( 0.017666f,  0.002666f),
        vec2(-0.017666f,  0.002666f),
        vec2(-0.017666f, -0.002666f),
        vec2( 0.017666f, -0.002666f),
        vec2( 0.017666f,  0.002666f),

        // Vertical Quad:
        // X is just the thickness
        // Y goes from -(height + thickness) to +(height + thickness)
        vec2(-0.0015f, -0.02816f),
        vec2( 0.0015f, -0.02816f),
        vec2( 0.0015f,  0.02816f),
        vec2(-0.0015f, -0.02816f),
        vec2( 0.0015f,  0.02816f),
        vec2(-0.0015f,  0.02816f)
    );
    gl_Position = vec4(corners[gl_VertexID],0.0,1.0);
}