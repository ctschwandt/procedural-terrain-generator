#version 420 core

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 light_dir;
// Matches Terrain::ColorScheme: Grayscale, Canyon, FrozenTundra, Volcanic.
uniform int color_scheme;

out vec4 FragColor;

// A sin-free hash avoids precision-dependent patterns on some OpenGL drivers.
float hash21(vec2 p)
{
    vec3 p3 = fract(vec3(p.x, p.y, p.x) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float value_noise(vec2 p)
{
    vec2 cell = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash21(cell);
    float b = hash21(cell + vec2(1.0, 0.0));
    float c = hash21(cell + vec2(0.0, 1.0));
    float d = hash21(cell + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

vec3 altitude_gradient(float t, vec3 low, vec3 mid_low, vec3 mid_high, vec3 high)
{
    vec3 color = mix(low, mid_low, smoothstep(0.05, 0.40, t));
    color = mix(color, mid_high, smoothstep(0.38, 0.72, t));
    return mix(color, high, smoothstep(0.70, 0.96, t));
}

vec3 choose_terrain_base_color(float height, float slope, vec2 world_xz, out float lava_emission)
{
    // Retain the original grayscale mapping; use a narrower biome range to
    // reveal the other palettes even with default +/-18-to-20-unit terrain.
    float gray_t = clamp((height + 64.0) / 128.0, 0.0, 1.0);
    float biome_t = smoothstep(-25.0, 32.0, height);
    float cliff = smoothstep(0.22, 0.68, slope);
    lava_emission = 0.0;

    if (color_scheme == 0) // Grayscale: the original default appearance
    {
        return mix(vec3(0.20, 0.20, 0.22),
                   vec3(0.88, 0.88, 0.90), gray_t);
    }

    // Smooth, spatially varying detail instead of per-pixel random speckling.
    float broad = value_noise(world_xz * 0.065);
    float detail = value_noise(world_xz * 0.34);
    float variation = broad * 0.65 + detail * 0.35;

    if (color_scheme == 1) // Canyon: warm sandstone and geological strata
    {
        vec3 sandstone = altitude_gradient(biome_t,
            vec3(0.28, 0.11, 0.075), // deep red clay
            vec3(0.57, 0.23, 0.12),  // red sandstone
            vec3(0.82, 0.43, 0.22),  // ochre
            vec3(0.92, 0.72, 0.49)); // sunlit pale sandstone

        float warped_height = height + (broad - 0.5) * 3.5;
        float bands = 0.5 + 0.5 * sin(warped_height * 1.6);
        sandstone *= mix(0.86, 1.10, smoothstep(0.22, 0.82, bands));
        sandstone = mix(sandstone, vec3(0.42, 0.18, 0.12), cliff * 0.30);
        return sandstone * mix(0.94, 1.06, variation);
    }

    if (color_scheme == 2) // Frozen Tundra: granite, glacial ice and snowcaps
    {
        vec3 rock_and_ice = altitude_gradient(biome_t,
            vec3(0.15, 0.24, 0.28), // blue-gray rock
            vec3(0.33, 0.49, 0.56), // glacial teal
            vec3(0.61, 0.77, 0.84), // pale ice
            vec3(0.88, 0.94, 0.98));

        float snowline = 6.0 + (broad - 0.5) * 12.0;
        float snow = smoothstep(snowline - 7.0, snowline + 8.0, height);
        snow *= 1.0 - cliff * 0.75; // steep faces expose bare rock and blue ice
        vec3 snow_color = vec3(0.91, 0.95, 0.99);
        rock_and_ice = mix(rock_and_ice, snow_color, snow * 0.90);
        rock_and_ice = mix(rock_and_ice, vec3(0.31, 0.45, 0.52), cliff * 0.28);
        return rock_and_ice * mix(0.96, 1.04, detail);
    }

    // Volcanic: charcoal basalt with glowing lava in lowland fissures.
    vec3 basalt = altitude_gradient(biome_t,
        vec3(0.10, 0.075, 0.07),
        vec3(0.13, 0.13, 0.15),
        vec3(0.24, 0.22, 0.23),
        vec3(0.40, 0.36, 0.34));
    basalt *= mix(0.83, 1.12, variation);
    basalt = mix(basalt, vec3(0.085, 0.08, 0.09), cliff * 0.20);

    float lowland = 1.0 - smoothstep(-7.0, 14.0, height);
    float fissure = 1.0 - smoothstep(0.025, 0.095, abs(broad - 0.50));
    float lava = lowland * max(fissure * 0.88, smoothstep(0.63, 0.85, variation) * 0.48);
    lava_emission = clamp(lava, 0.0, 1.0);
    return mix(basalt, vec3(1.0, 0.28, 0.035), lava_emission * 0.94);
}

void main()
{
    const float ambient_strength = 0.30;
    const float diffuse_strength = 0.70;

    vec3 normal = normalize(vNormal);
    vec3 light = normalize(light_dir);
    float height = vWorldPos.y;
    float slope = 1.0 - clamp(normal.y, 0.0, 1.0);

    float lava_emission;
    vec3 base_color = choose_terrain_base_color(height, slope, vWorldPos.xz, lava_emission);
    float diffuse = max(dot(normal, light), 0.0);
    float lighting = ambient_strength + diffuse * diffuse_strength;

    // Lava stays vivid in shadow; all other surfaces retain directional shading.
    vec3 lit_color = mix(base_color * lighting, base_color, lava_emission * 0.88);
    FragColor = vec4(lit_color, 1.0);
}
