#version 460 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec2 FragVelocity;

in vec4 currentClipPos;
in vec4 previousClipPos;
in vec3 worldPos;           // world position from vertex shader

uniform float time;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    float speed = 0.5;
    float scale = 1.0; // 1.0 = one full rainbow per 1 world unit (tune to your block size)

    // Sum all axes so every edge direction (X, Y, Z) contributes color variation
    float hue = fract((worldPos.x + worldPos.y + worldPos.z) * scale + time * speed);

    vec3 rgbColor = hsv2rgb(vec3(hue, 1.0, 1.0));
    FragColor = vec4(rgbColor, 1.0);

    vec2 currNDC = currentClipPos.xy / currentClipPos.w;
    vec2 prevNDC = previousClipPos.xy / previousClipPos.w;
    FragVelocity = (currNDC - prevNDC) * 0.5;
}