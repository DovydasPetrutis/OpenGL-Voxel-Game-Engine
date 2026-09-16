#version 460 core
#extension GL_ARB_bindless_texture : require
in vec2 texCoord;
out vec4 f_color;

layout(bindless_sampler) uniform sampler2D u_MSDFtexture;
uniform vec3 u_color;
uniform float u_pxRange;
uniform vec2 u_textureSize;
uniform float time;

vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float median(vec3 rgb) {
  return max(min(rgb.r, rgb.g), min(max(rgb.r, rgb.g), rgb.b));
}

void main() {
  float speed = 0.5;
  float scale = 0.005; // Adjust this to make the rainbow "wider" or "thinner"
  float hue = fract((gl_FragCoord.x * scale) + (time * speed));

  // 1. Calculate your animated rainbow color
  vec3 rgbColor = hsv2rgb(vec3(hue, 1.0, 1.0));

  // 2. Sample the multi-channel distance field
  vec3 msd = texture(u_MSDFtexture, texCoord).rgb;
  float sd = median(msd);

  // 3. CORRECTED MSDF SCREEN-SPACE MATH:
  // fwidth(texCoord) gives UV change per screen pixel.
  // Multiplying by u_textureSize converts it to texture pixels (texels) per
  // screen pixel.
  vec2 texelDelta = fwidth(texCoord) * u_textureSize;
  float screenPixelsPerTexel = 1.0 / length(texelDelta);

  // Calculate final clean distance in screen pixels
  float screenPxDistance = u_pxRange * (sd - 0.5) * screenPixelsPerTexel;

  // Smooth anti-aliased edge
  float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

  if (opacity < 0.01)
    discard;

  f_color = vec4(rgbColor, opacity);
}