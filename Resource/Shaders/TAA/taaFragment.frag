#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D currentFrame;
uniform sampler2D previousFrame;
uniform sampler2D currentVelocityFrame;
uniform sampler2D previousVelocityFrame;
uniform sampler2D depthBuffer;
uniform float stddevss;
uniform vec2 resolution;
uniform vec2 jitterUV;
uniform vec2 prevJitterUV;

// ============================================================
// Color space helpers
// ============================================================

vec3 RGBToYCoCg(vec3 rgb)
{
    return vec3(
         0.25 * rgb.r + 0.5 * rgb.g + 0.25 * rgb.b,   // Y
         0.5  * rgb.r               - 0.5  * rgb.b,    // Co
        -0.25 * rgb.r + 0.5 * rgb.g - 0.25 * rgb.b    // Cg
    );
}

vec3 YCoCgToRGB(vec3 y)
{
    return vec3(
        y.x + y.y - y.z,
        y.x        + y.z,
        y.x - y.y - y.z
    );
}

// ============================================================
// Depth-dilated velocity (3×3 closest-depth neighbour)
// ============================================================

vec2 getDilatedVelocity()
{
    vec2 px = 1.0 / resolution;
    float closestDepth    = 0.0;
    vec2  closestVelocity = texture(currentVelocityFrame, TexCoords).rg;
    for (int x = -1; x <= 1; x++)
    for (int y = -1; y <= 1; y++)
    {
        vec2  off = vec2(x, y) * px;
        float d   = texture(depthBuffer, TexCoords + off).r;
        if (d > closestDepth)
        {
            closestDepth    = d;
            closestVelocity = texture(currentVelocityFrame, TexCoords + off).rg;
        }
    }
    return closestVelocity;
}

// ============================================================
// 9-tap Catmull-Rom bicubic for history reprojection.
// ============================================================

vec3 SampleCatmullRom(sampler2D tex, vec2 uv, vec2 texSize)
{
    vec2 sp  = uv * texSize;
    vec2 tc1 = floor(sp - 0.5) + 0.5;
    vec2 f   = sp - tc1;
    vec2 w0  = f * (-0.5 + f * (1.0 - 0.5 * f));
    vec2 w1  = 1.0 + f * f * (-2.5 + 1.5 * f);
    vec2 w2  = f * (0.5 + f * (2.0 - 1.5 * f));
    vec2 w3  = f * f * (-0.5 + 0.5 * f);
    vec2 w12 = w1 + w2;
    vec2 off = w2 / w12;

    vec2 p0  = (tc1 - 1.0) / texSize;
    vec2 p3  = (tc1 + 2.0) / texSize;
    vec2 p12 = (tc1 + off) / texSize;

    vec3 r = vec3(0.0);
    r += texture(tex, vec2(p0.x,  p0.y )).rgb * w0.x  * w0.y;
    r += texture(tex, vec2(p12.x, p0.y )).rgb * w12.x * w0.y;
    r += texture(tex, vec2(p3.x,  p0.y )).rgb * w3.x  * w0.y;
    r += texture(tex, vec2(p0.x,  p12.y)).rgb * w0.x  * w12.y;
    r += texture(tex, vec2(p12.x, p12.y)).rgb * w12.x * w12.y;
    r += texture(tex, vec2(p3.x,  p12.y)).rgb * w3.x  * w12.y;
    r += texture(tex, vec2(p0.x,  p3.y )).rgb * w0.x  * w3.y;
    r += texture(tex, vec2(p12.x, p3.y )).rgb * w12.x * w3.y;
    r += texture(tex, vec2(p3.x,  p3.y )).rgb * w3.x  * w3.y;
    return r;
}

// ============================================================
// Variance-clip history to the neighbourhood AABB.
// ============================================================

vec3 ClipToAABB(vec3 prevColor, vec3 clipTarget, vec3 aabbMin, vec3 aabbMax)
{
    if (all(greaterThanEqual(prevColor, aabbMin)) &&
        all(lessThanEqual  (prevColor, aabbMax)))
        return prevColor;
    vec3 dir = clipTarget - prevColor;

    const float kEps = 1e-7;
    vec3 safeDir = mix(sign(dir + vec3(1e-9)) * vec3(kEps), dir, step(vec3(kEps), abs(dir)));
    vec3 invDir  = 1.0 / safeDir;
    vec3 t0 = (aabbMin - prevColor) * invDir;
    vec3 t1 = (aabbMax - prevColor) * invDir;

    float tEnter = max(max(min(t0.x, t1.x),
                            min(t0.y, t1.y)),
                            min(t0.z, t1.z));
    float tExit  = min(min(max(t0.x, t1.x),
                            max(t0.y, t1.y)),
                            max(t0.z, t1.z));
    float t = clamp(tEnter, 0.0, min(tExit, 1.0));
    return prevColor + dir * t;
}

// ============================================================
// Main
// ============================================================

void main()
{
    // ----------------------------------------------------------
    // 1. Reprojection via dilated velocity
    // ----------------------------------------------------------
    vec2 velocity      = getDilatedVelocity();
    vec2 reprojectedUV = TexCoords - velocity;

    bool outOfBounds = any(lessThan   (reprojectedUV, vec2(0.0))) ||
                       any(greaterThan(reprojectedUV, vec2(1.0)));

    // ----------------------------------------------------------
    // 2. Build YCoCg neighbourhood statistics.
    // ----------------------------------------------------------
    ivec2 center = ivec2(gl_FragCoord.xy);
    vec3  m1     = vec3(0.0);
    vec3  m2     = vec3(0.0);
    vec3  minC   = vec3( 1e9);
    vec3  maxC   = vec3(-1e9);

    // 3×3 loop for variance statistics only.
    for (int x = -1; x <= 1; x++)
    for (int y = -1; y <= 1; y++)
    {
        vec3 s = RGBToYCoCg(texelFetch(currentFrame, center + ivec2(x, y), 0).rgb);
        m1 += s;
        m2 += s * s;
    }

    // 5-tap cross for hard AABB bounds (more stable under jitter).
    {
        vec3 s;
        s = RGBToYCoCg(texelFetch(currentFrame, center,                  0).rgb);
        minC = min(minC,s); maxC = max(maxC,s);
        s = RGBToYCoCg(texelFetch(currentFrame, center+ivec2( 1, 0), 0).rgb); minC = min(minC,s); maxC = max(maxC,s);
        s = RGBToYCoCg(texelFetch(currentFrame, center+ivec2(-1, 0), 0).rgb); minC = min(minC,s); maxC = max(maxC,s);
        s = RGBToYCoCg(texelFetch(currentFrame, center+ivec2( 0, 1), 0).rgb); minC = min(minC,s); maxC = max(maxC,s);
        s = RGBToYCoCg(texelFetch(currentFrame, center+ivec2( 0,-1), 0).rgb); minC = min(minC,s); maxC = max(maxC,s);
    }

    // ----------------------------------------------------------
    // [UPDATED] Motion-Adaptive Clamping Bounds
    // ----------------------------------------------------------
    vec3 mean   = m1 / 9.0;
    vec3 stddev = sqrt(max(m2 / 9.0 - mean * mean, vec3(0.0)));

    // 1. Calculate the current screen-space velocity in pixels
    float speedPx = length(velocity * resolution);

    // 2. Map speed to a factor between 0.0 (still) and 1.0 (moving more than 2 pixels)
    float motionFactor = clamp(speedPx / 2.0, 0.0, 1.0);

    // 3. Scale the variance sigma. When still, we open up to 3.5 sigma to let the 
    //    jitter accumulate flawlessly. When moving, we tighten it down to stddevss.
    float currentStdDevSS = mix(3.0, stddevss, motionFactor);

    // 4. Generate the base relaxed variance bounds
    vec3 varMin = mean - currentStdDevSS * stddev;
    vec3 varMax = mean + currentStdDevSS * stddev;

    // 5. Smoothly blend in the strict 5-tap cross (minC/maxC) ONLY when moving.
    //    This completely removes the jitter-induced color box oscillation while stationary.
    varMin = mix(varMin, max(varMin, minC), motionFactor);
    varMax = mix(varMax, min(varMax, maxC), motionFactor);

    // ----------------------------------------------------------
    // 3. History sample (Catmull-Rom bicubic)
    // ----------------------------------------------------------
    // You already computed speedPx above — reuse it.
    vec3 historyCR    = SampleCatmullRom(previousFrame, reprojectedUV, resolution);
    vec3 historyBilin = texture(previousFrame, reprojectedUV).rgb;   // history FBO is GL_LINEAR → free bilinear

    // 0 px = full sharp Catmull-Rom (still), 8+ px = full bilinear (moving, no ringing)
    float crToBilin = clamp(speedPx / 8.0, 0.0, 1.0);
    vec3 historySample = max(mix(historyCR, historyBilin, crToBilin), vec3(0.0));
    vec3 historyYCoCg  = RGBToYCoCg(historySample);

    // ----------------------------------------------------------
    // 4. Variance-clip history toward the neighbourhood MEAN.
    // ----------------------------------------------------------
    vec3 clippedHistoryYCoCg = ClipToAABB(historyYCoCg, mean, varMin, varMax);
    vec3 clippedHistory      = YCoCgToRGB(clippedHistoryYCoCg);

    // ----------------------------------------------------------
    // 5. Velocity disocclusion rejection
    // ----------------------------------------------------------
    vec2  prevVelocity         = texture(previousVelocityFrame, reprojectedUV).rg;
    float velDiff      = length(prevVelocity - velocity);
    // Softened multiplier from 20.0 to 10.0 and raised the threshold
    float velocityDisocclusion = clamp((velDiff - 0.01) * 8.0, 0.0, 1.0);

    // ----------------------------------------------------------
    // 6. Blend factor
    // ----------------------------------------------------------
    // Note: speedPx type declaration removed here to prevent GLSL compiler error
    float motionBlend = 0.1;
    float blendFactor = outOfBounds
                    ? 1.0
                    : mix(motionBlend, 1.0, velocityDisocclusion);

    // ----------------------------------------------------------
    // 7. Final accumulation
    // ----------------------------------------------------------


    vec3 currentRGB   = texelFetch(currentFrame, center, 0).rgb;
    vec3 accumulation = mix(clippedHistory, currentRGB, blendFactor);
    FragColor = vec4(max(accumulation, vec3(0.0)), 1.0);
    // FragColor = vec4(max(accumulation, vec3(0.0)), 1.0);
}