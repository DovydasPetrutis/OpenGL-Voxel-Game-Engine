#version 460 core

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D currentFrame;
uniform float sharpness;
uniform bool cas;
uniform sampler2D previousHistory;

vec3 CAS(sampler2D tex, vec2 uv, float sharpness)
{
    vec3 b = textureOffset(tex, uv, ivec2( 0, -1)).rgb;
    vec3 c = textureOffset(tex, uv, ivec2(-1,  0)).rgb;
    vec3 d = texture(tex, uv).rgb;
    vec3 e = textureOffset(tex, uv, ivec2( 1,  0)).rgb;
    vec3 f = textureOffset(tex, uv, ivec2( 0,  1)).rgb;

    vec3 mnRGB = min(min(min(b, c), min(d, e)), f);
    vec3 mxRGB = max(max(max(b, c), max(d, e)), f);

    vec3 ampRGB = clamp(min(mnRGB, 2.0 - mxRGB) / (mxRGB + 0.0001), 0.0, 1.0);
    ampRGB = sqrt(ampRGB);

    float peak = -1.0 / mix(8.0, 5.0, clamp(sharpness, 0.0, 1.0));
    vec3 w = ampRGB * peak;

    return clamp((b * w + c * w + d + e * w + f * w) / (4.0 * w + 1.0), 0.0, 1.0);
}

void main()
{
    if(cas)
    {
        FragColor = vec4(CAS(currentFrame,TexCoords,sharpness),1.0); 
    }
    else
    {
        FragColor = texture(currentFrame,TexCoords);
    }
}