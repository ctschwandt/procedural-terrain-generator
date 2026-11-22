#ifndef TERRAIN_H
#define TERRAIN_H

#include "camera.h"
#include "mesh.h"
#include "shader.h"

#include <array>
#include <vector>

class Terrain
{
public:
    enum class Mode
    {
        Sinusoidal,
        DiamondSquare,
        Perlin,
        Simplex
    };

    enum class ColorScheme
    {
        Grayscale = 0,
        Canyon = 1,
        FrozenTundra = 2,
        Volcanic = 3
    };

    struct SinusoidalParams
    {
        float amplitude;
        float x_frequency;
        float z_frequency;
    };

    struct PerlinParams
    {
        float height_scale;
        float base_frequency;
        int octaves;
        float persistence;
        float lacunarity;
        float redistribution_power;
        int seed;
    };

    struct SimplexParams
    {
        float height_scale;
        float base_frequency;
        int octaves;
        float persistence;
        float lacunarity;
        float redistribution_power;
        int seed;
    };

    struct DiamondSquareParams
    {
        float initial_amplitude;
        float roughness;
        int seed;
        int exponent;
        float spacing;
    };

    Terrain();

    bool init();
    void update(const Camera& camera);
    void render(double now, int width, int height, const Camera& camera, bool wireframe_enabled);
    void shutdown();

    void set_mode(Mode new_mode);
    Mode get_mode() const;
    void set_color_scheme(ColorScheme new_color_scheme);
    ColorScheme get_color_scheme() const;
    float get_height_at_world(float world_x, float world_z) const;

    void reset_active_mode_params_to_defaults();
    void rebuild_active_mode();
    const SinusoidalParams& get_sinusoidal_params() const;
    const PerlinParams& get_perlin_params() const;
    const SimplexParams& get_simplex_params() const;
    const DiamondSquareParams& get_diamond_square_params() const;
    void set_sinusoidal_params(const SinusoidalParams& params);
    void set_perlin_params(const PerlinParams& params);
    void set_simplex_params(const SimplexParams& params);
    void set_diamond_square_params(const DiamondSquareParams& params);

private:
    static constexpr int fixed_patch_cells = 512;
    static constexpr float cell_size = 1.0f;

    Mesh fixed_patch_mesh;
    Mesh diamond_square_mesh;
    std::vector<float> diamond_square_heights;
    int diamond_square_size;
    float diamond_square_spacing;
    float diamond_square_half_span;
    Shader shader;
    Mode active_mode;
    ColorScheme active_color_scheme;

    SinusoidalParams sinusoidal_params;
    PerlinParams perlin_params;
    SimplexParams simplex_params;
    DiamondSquareParams diamond_square_params;
    std::array<int, 512> perlin_permutation;
    std::array<int, 512> simplex_permutation;

    static SinusoidalParams default_sinusoidal_params();
    static PerlinParams default_perlin_params();
    static SimplexParams default_simplex_params();
    static DiamondSquareParams default_diamond_square_params();

    static const char* mode_to_string(Mode mode);
    float sample_sinusoidal_height(float world_x, float world_z) const;
    static float sample_fractal_perlin_2d(float world_x,
                                          float world_z,
                                          float base_frequency,
                                          int octaves,
                                          float persistence,
                                          float lacunarity,
                                          float offset_x,
                                          float offset_z,
                                          const std::array<int, 512>& permutation);
    float sample_perlin_elevation01(float world_x, float world_z) const;
    float sample_perlin_height(float world_x, float world_z) const;
    float sample_simplex_height(float world_x, float world_z) const;
    float sample_simplex_elevation01(float world_x, float world_z) const;
    static float sample_fractal_simplex_2d(float world_x,
                                           float world_z,
                                           float base_frequency,
                                           int octaves,
                                           float persistence,
                                           float lacunarity,
                                           float offset_x,
                                           float offset_z,
                                           const std::array<int, 512>& permutation);
    float sample_height(float world_x, float world_z) const;
    void build_fixed_patch_mesh();

    static void append_grid_indices(int grid_cells, std::vector<unsigned int>& indices);
    static void build_vertex_normals(const std::vector<Vec3>& positions,
                                     const std::vector<unsigned int>& indices,
                                     std::vector<Vec3>& normals);
    static void build_interleaved_vertex_data(const std::vector<Vec3>& positions,
                                              const std::vector<Vec3>& normals,
                                              const std::vector<Vec3>* colors,
                                              std::vector<float>& vertices);

    void build_diamond_square_mesh();
    void rebuild_for_active_mode();
};

#endif
