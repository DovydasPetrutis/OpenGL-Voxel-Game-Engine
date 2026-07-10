#version 460 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec2 FragVelocity;

in vec3 aTexCoords; 
uniform sampler2DArray ourTexture;
uniform bool TAA;

in vec4 currentClipPos;
in vec4 previousClipPos;


void main()
{    
    // Use the smooth-nearest sampling instead of standard texture()
    vec4 albedo = texture(ourTexture, aTexCoords);

    if(TAA)
    {
        FragColor = albedo;
        
        vec2 currNDC = currentClipPos.xy / currentClipPos.w;
        vec2 prevNDC = previousClipPos.xy / previousClipPos.w;
        
        vec2 velocity = currNDC - prevNDC;
        // Velocity stored as 32-bit floats
        FragVelocity = velocity * 0.5;
    }
    else
    {
        FragColor = albedo;
    }
}