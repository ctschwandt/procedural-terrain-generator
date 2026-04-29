#version 460 core

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec3 light_dir;

out vec4 FragColor;

vec3 choose_terrain_base_color(float height, float slope)
{
    const float min_height = -64.0;
    const float max_height = 64.0;
    float t = clamp((height - min_height) / (max_height - min_height), 0.0, 1.0);
    vec3 dark_gray = vec3(0.20, 0.20, 0.22);
    vec3 light_gray = vec3(0.88, 0.88, 0.90);
    vec3 base_color = mix(dark_gray, light_gray, t);

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
