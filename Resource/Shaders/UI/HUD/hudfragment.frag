#version 460 core

uniform float time; // Pass this from your code
out vec4 FragColor;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    float speed = 0.5;
    float scale = 0.005; // Adjust this to make the rainbow "wider" or "thinner"
    float hue = fract((gl_FragCoord.x * scale) + (time * speed));
    
    vec3 rgbColor = hsv2rgb(vec3(hue, 1.0, 1.0));
    
    FragColor = vec4(rgbColor, 1.0);
}
