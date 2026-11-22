#version 460 core

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 light_dir;
// Matches Terrain::ColorScheme: NaturalMountain, Canyon, FrozenTundra, Grayscale, Volcanic.
uniform int color_scheme;

out vec4 FragColor;

vec3 altitude_gradient(float t, vec3 low, vec3 mid_low, vec3 mid_high, vec3 high)
{
    vec3 color = mix(low, mid_low, smoothstep(0.05, 0.40, t));
    color = mix(color, mid_high, smoothstep(0.38, 0.72, t));
    return mix(color, high, smoothstep(0.70, 0.96, t));
}

vec3 choose_terrain_base_color(float height, float slope)
{
    // Keep the original elevation mapping, shared across terrain-generation modes.
    const float min_height = -64.0;
    const float max_height = 64.0;
    float t = clamp((height - min_height) / (max_height - min_height), 0.0, 1.0);
    float cliff = smoothstep(0.28, 0.72, slope);
    vec3 base_color;

    if (color_scheme == 0) // Natural Mountain: grass, earth, rock, snow
    {
        base_color = altitude_gradient(t,
            vec3(0.09, 0.21, 0.10),
            vec3(0.25, 0.40, 0.19),
            vec3(0.48, 0.45, 0.39),
            vec3(0.94, 0.95, 0.95));
        base_color = mix(base_color, vec3(0.43, 0.43, 0.41), cliff * 0.55);
    }
    else if (color_scheme == 1) // Canyon: sandstone and layered red rock
    {
        base_color = altitude_gradient(t,
            vec3(0.24, 0.10, 0.07),
            vec3(0.57, 0.23, 0.12),
            vec3(0.81, 0.46, 0.25),
            vec3(0.94, 0.72, 0.45));
        float strata = 0.5 + 0.5 * sin(height * 0.65);
        base_color *= mix(0.90, 1.08, strata);
    }
    else if (color_scheme == 2) // Frozen Tundra: cold stone, ice, snow
    {
        base_color = altitude_gradient(t,
            vec3(0.11, 0.20, 0.23),
            vec3(0.29, 0.40, 0.43),
            vec3(0.69, 0.81, 0.87),
            vec3(0.97, 0.98, 1.00));
        base_color = mix(base_color, vec3(0.38, 0.48, 0.54), cliff * 0.40);
    }
    else if (color_scheme == 3) // Grayscale: original elevation-based appearance
    {
        base_color = mix(vec3(0.20, 0.20, 0.22),
                         vec3(0.88, 0.88, 0.90), t);
    }
    else // Volcanic: glowing lowlands, basalt, ash
    {
        base_color = altitude_gradient(t,
            vec3(0.16, 0.08, 0.07),
            vec3(0.12, 0.11, 0.13),
            vec3(0.27, 0.25, 0.27),
            vec3(0.47, 0.44, 0.43));
        float lava = 1.0 - smoothstep(0.30, 0.46, t);
        base_color = mix(base_color, vec3(0.97, 0.24, 0.025), lava * 0.85);
    }

    float slope_darkening = smoothstep(0.22, 0.75, slope) * 0.18;
    return base_color * (1.0 - slope_darkening);
}

void main()
{
    const float ambient_strength = 0.30;
    const float diffuse_strength = 0.70;

    vec3 normal = normalize(vNormal);
    vec3 light = normalize(light_dir);

    float height = vWorldPos.y;
    float slope = 1.0 - clamp(normal.y, 0.0, 1.0);
    vec3 base_color = choose_terrain_base_color(height, slope);

    float diffuse = max(dot(normal, light), 0.0);
    float lighting = ambient_strength + diffuse * diffuse_strength;

    vec3 lit_color = base_color * lighting;
    FragColor = vec4(lit_color, 1.0);
}
