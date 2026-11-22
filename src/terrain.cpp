#include "terrain.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <vector>

#include "math.h"

namespace
{
static constexpr std::array<int, 256> perlin_base_permutation = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
    140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
    247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
    57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
    74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
    60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
    65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
    200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
    52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
    207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
    119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
    218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
    81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
    184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
    222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};

static std::array<int, 512> make_seeded_permutation(int seed)
{
    std::array<int, 256> shuffled = perlin_base_permutation;
    std::mt19937 rng(static_cast<unsigned int>(seed));
    std::shuffle(shuffled.begin(), shuffled.end(), rng);

    std::array<int, 512> permutation = {};
    for (int i = 0; i < 512; ++i)
    {
        permutation[static_cast<std::size_t>(i)] = shuffled[static_cast<std::size_t>(i & 255)];
    }
    return permutation;
}

static float perlin_fade(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static float perlin_lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

static float perlin_gradient(int hash, float x, float z)
{
    int h = hash & 7;
    float u = (h < 4) ? x : z;
    float v = (h < 4) ? z : x;
    float u_signed = ((h & 1) == 0) ? u : -u;
    float v_signed = ((h & 2) == 0) ? v : -v;
    return u_signed + v_signed;
}

static float perlin_noise_2d(float x, float z, const std::array<int, 512>& permutation)
{
    int xi = static_cast<int>(std::floor(x)) & 255;
    int zi = static_cast<int>(std::floor(z)) & 255;

    float xf = x - std::floor(x);
    float zf = z - std::floor(z);

    float u = perlin_fade(xf);
    float v = perlin_fade(zf);

    int aa = permutation[static_cast<std::size_t>(permutation[static_cast<std::size_t>(xi)] + zi)];
    int ab = permutation[static_cast<std::size_t>(permutation[static_cast<std::size_t>(xi)] + zi + 1)];
    int ba = permutation[static_cast<std::size_t>(permutation[static_cast<std::size_t>(xi + 1)] + zi)];
    int bb = permutation[static_cast<std::size_t>(permutation[static_cast<std::size_t>(xi + 1)] + zi + 1)];

    float x1 = perlin_lerp(perlin_gradient(aa, xf, zf), perlin_gradient(ba, xf - 1.0f, zf), u);
    float x2 = perlin_lerp(perlin_gradient(ab, xf, zf - 1.0f), perlin_gradient(bb, xf - 1.0f, zf - 1.0f), u);
    return perlin_lerp(x1, x2, v);
}

static constexpr float perlin_elevation_offset_x = 113.0f;
static constexpr float perlin_elevation_offset_z = 271.0f;

static const std::array<Vec3, 12> simplex_gradients_2d = {
    v3( 1.0f,  1.0f, 0.0f),
    v3(-1.0f,  1.0f, 0.0f),
    v3( 1.0f, -1.0f, 0.0f),
    v3(-1.0f, -1.0f, 0.0f),
    v3( 1.0f,  0.0f, 0.0f),
    v3(-1.0f,  0.0f, 0.0f),
    v3( 1.0f,  0.0f, 0.0f),
    v3(-1.0f,  0.0f, 0.0f),
    v3( 0.0f,  1.0f, 0.0f),
    v3( 0.0f, -1.0f, 0.0f),
    v3( 0.0f,  1.0f, 0.0f),
    v3( 0.0f, -1.0f, 0.0f)
};

static float simplex_grad_dot(int hash, float x, float z)
{
    const Vec3& g = simplex_gradients_2d[static_cast<std::size_t>(hash % 12)];
    return g.x * x + g.y * z;
}

static float simplex_noise_2d(float x, float z, const std::array<int, 512>& permutation)
{
    static constexpr float F2 = 0.3660254037844386f;
    static constexpr float G2 = 0.2113248654051871f;

    float s = (x + z) * F2;
    int i = static_cast<int>(std::floor(x + s));
    int j = static_cast<int>(std::floor(z + s));

    float t = static_cast<float>(i + j) * G2;
    float cell_origin_x = static_cast<float>(i) - t;
    float cell_origin_z = static_cast<float>(j) - t;

    float x0 = x - cell_origin_x;
    float z0 = z - cell_origin_z;

    int i1 = 0;
    int j1 = 0;
    if (x0 > z0)
    {
        i1 = 1;
        j1 = 0;
    }
    else
    {
        i1 = 0;
        j1 = 1;
    }

    float x1 = x0 - static_cast<float>(i1) + G2;
    float z1 = z0 - static_cast<float>(j1) + G2;
    float x2 = x0 - 1.0f + 2.0f * G2;
    float z2 = z0 - 1.0f + 2.0f * G2;

    int ii = i & 255;
    int jj = j & 255;

    int gi0 = permutation[static_cast<std::size_t>(ii + permutation[static_cast<std::size_t>(jj)])] % 12;
    int gi1 = permutation[static_cast<std::size_t>(ii + i1 + permutation[static_cast<std::size_t>(jj + j1)])] % 12;
    int gi2 = permutation[static_cast<std::size_t>(ii + 1 + permutation[static_cast<std::size_t>(jj + 1)])] % 12;

    float n0 = 0.0f;
    float n1 = 0.0f;
    float n2 = 0.0f;

    float t0 = 0.5f - x0 * x0 - z0 * z0;
    if (t0 > 0.0f)
    {
        t0 *= t0;
        n0 = t0 * t0 * simplex_grad_dot(gi0, x0, z0);
    }

    float t1 = 0.5f - x1 * x1 - z1 * z1;
    if (t1 > 0.0f)
    {
        t1 *= t1;
        n1 = t1 * t1 * simplex_grad_dot(gi1, x1, z1);
    }

    float t2 = 0.5f - x2 * x2 - z2 * z2;
    if (t2 > 0.0f)
    {
        t2 *= t2;
        n2 = t2 * t2 * simplex_grad_dot(gi2, x2, z2);
    }

    return 70.0f * (n0 + n1 + n2);
}

static constexpr float simplex_elevation_offset_x = 419.0f;
static constexpr float simplex_elevation_offset_z = 137.0f;
}

Terrain::SinusoidalParams Terrain::default_sinusoidal_params()
{
    SinusoidalParams params = {};
    params.amplitude = 18.0f;
    params.x_frequency = 0.15f;
    params.z_frequency = 0.15f;
    return params;
}

Terrain::PerlinParams Terrain::default_perlin_params()
{
    PerlinParams params = {};
    params.height_scale = 20.0f;
    params.base_frequency = 0.018f;
    params.octaves = 6;
    params.persistence = 0.5f;
    params.lacunarity = 2.0f;
    params.redistribution_power = 3.8f;
    params.seed = 1337;
    return params;
}

Terrain::DiamondSquareParams Terrain::default_diamond_square_params()
{
    DiamondSquareParams params = {};
    params.initial_amplitude = 48.0f;
    params.roughness = 0.55f;
    params.seed = 1337;
    params.exponent = 7;
    params.spacing = 3.0f;
    return params;
}

Terrain::SimplexParams Terrain::default_simplex_params()
{
    SimplexParams params = {};
    params.height_scale = 20.0f;
    params.base_frequency = 0.018f;
    params.octaves = 6;
    params.persistence = 0.5f;
    params.lacunarity = 2.0f;
    params.redistribution_power = 3.8f;
    params.seed = 7331;
    return params;
}

Terrain::Terrain()
    : diamond_square_size(0)
    , diamond_square_spacing(1.0f)
    , diamond_square_half_span(0.0f)
    , active_mode(Mode::Sinusoidal)
    , active_color_scheme(ColorScheme::Grayscale)
    , sinusoidal_params(default_sinusoidal_params())
    , perlin_params(default_perlin_params())
    , simplex_params(default_simplex_params())
    , diamond_square_params(default_diamond_square_params())
    , perlin_permutation(make_seeded_permutation(default_perlin_params().seed))
    , simplex_permutation(make_seeded_permutation(default_simplex_params().seed))
{
}

const char* Terrain::mode_to_string(Mode mode)
{
    if (mode == Mode::Sinusoidal)
    {
        return "Sinusoidal";
    }
    if (mode == Mode::DiamondSquare)
    {
        return "DiamondSquare";
    }
    if (mode == Mode::Perlin)
    {
        return "Perlin";
    }
    return "Simplex";
}

bool Terrain::init()
{
    if (!shader.load_from_files("shaders/terrain.vert", "shaders/terrain.frag"))
    {
        return false;
    }

    rebuild_for_active_mode();
    return true;
}

float Terrain::sample_sinusoidal_height(float world_x, float world_z) const
{
    return std::sin(world_x * sinusoidal_params.x_frequency)
         * std::cos(world_z * sinusoidal_params.z_frequency)
         * sinusoidal_params.amplitude;
}

float Terrain::sample_perlin_height(float world_x, float world_z) const
{
    float elevation01 = sample_perlin_elevation01(world_x, world_z);
    return elevation01 * perlin_params.height_scale;
}

float Terrain::sample_fractal_perlin_2d(float world_x,
                                        float world_z,
                                        float base_frequency,
                                        int octaves,
                                        float persistence,
                                        float lacunarity,
                                        float offset_x,
                                        float offset_z,
                                        const std::array<int, 512>& permutation)
{
    float frequency = base_frequency;
    float amplitude = 1.0f;
    float total = 0.0f;
    float amplitude_sum = 0.0f;

    for (int octave = 0; octave < octaves; ++octave)
    {
        float octave_offset_x = offset_x + static_cast<float>(octave) * 53.0f;
        float octave_offset_z = offset_z + static_cast<float>(octave) * 97.0f;
        float sample_x = world_x * frequency + octave_offset_x;
        float sample_z = world_z * frequency + octave_offset_z;
        float noise_sample = perlin_noise_2d(sample_x, sample_z, permutation);
        total += noise_sample * amplitude;
        amplitude_sum += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    if (amplitude_sum <= 0.0f)
    {
        return 0.0f;
    }

    return clampf(total / amplitude_sum, -1.0f, 1.0f);
}

float Terrain::sample_perlin_elevation01(float world_x, float world_z) const
{
    float elevation_raw = sample_fractal_perlin_2d(world_x,
                                                   world_z,
                                                   perlin_params.base_frequency,
                                                   perlin_params.octaves,
                                                   perlin_params.persistence,
                                                   perlin_params.lacunarity,
                                                   perlin_elevation_offset_x,
                                                   perlin_elevation_offset_z,
                                                   perlin_permutation);
    return clampf(elevation_raw * 0.5f + 0.5f, 0.0f, 1.0f);
}

float Terrain::sample_height(float world_x, float world_z) const
{
    if (active_mode == Mode::Perlin)
    {
        return sample_perlin_height(world_x, world_z);
    }

    if (active_mode == Mode::Simplex)
    {
        return sample_simplex_height(world_x, world_z);
    }

    return sample_sinusoidal_height(world_x, world_z);
}

float Terrain::get_height_at_world(float world_x, float world_z) const
{
    if (active_mode == Mode::Sinusoidal || active_mode == Mode::Perlin || active_mode == Mode::Simplex)
    {
        return sample_height(world_x, world_z);
    }

    if (diamond_square_heights.empty() || diamond_square_size <= 1 || diamond_square_spacing <= 0.0f)
    {
        return 0.0f;
    }

    float local_x = (world_x + diamond_square_half_span) / diamond_square_spacing;
    float local_z = (world_z + diamond_square_half_span) / diamond_square_spacing;

    float max_coord = static_cast<float>(diamond_square_size - 1);
    local_x = std::clamp(local_x, 0.0f, max_coord);
    local_z = std::clamp(local_z, 0.0f, max_coord);

    int x0 = static_cast<int>(std::floor(local_x));
    int z0 = static_cast<int>(std::floor(local_z));
    int x1 = std::min(x0 + 1, diamond_square_size - 1);
    int z1 = std::min(z0 + 1, diamond_square_size - 1);

    float tx = local_x - static_cast<float>(x0);
    float tz = local_z - static_cast<float>(z0);

    auto sample_ds = [this](int x, int z) -> float {
        return diamond_square_heights[static_cast<std::size_t>(z * diamond_square_size + x)];
    };

    float h00 = sample_ds(x0, z0);
    float h10 = sample_ds(x1, z0);
    float h01 = sample_ds(x0, z1);
    float h11 = sample_ds(x1, z1);

    float hx0 = h00 + (h10 - h00) * tx;
    float hx1 = h01 + (h11 - h01) * tx;
    return hx0 + (hx1 - hx0) * tz;
}

void Terrain::set_mode(Mode new_mode)
{
    if (active_mode == new_mode)
    {
        return;
    }

    active_mode = new_mode;
    rebuild_for_active_mode();
}

Terrain::Mode Terrain::get_mode() const
{
    return active_mode;
}

void Terrain::set_color_scheme(ColorScheme new_color_scheme)
{
    active_color_scheme = new_color_scheme;
}

Terrain::ColorScheme Terrain::get_color_scheme() const
{
    return active_color_scheme;
}

void Terrain::reset_active_mode_params_to_defaults()
{
    if (active_mode == Mode::Sinusoidal)
    {
        sinusoidal_params = default_sinusoidal_params();
    }
    else if (active_mode == Mode::Perlin)
    {
        perlin_params = default_perlin_params();
    }
    else if (active_mode == Mode::Simplex)
    {
        simplex_params = default_simplex_params();
    }
    else
    {
        diamond_square_params = default_diamond_square_params();
    }

    rebuild_for_active_mode();
}

void Terrain::rebuild_active_mode()
{
    rebuild_for_active_mode();
}

const Terrain::SinusoidalParams& Terrain::get_sinusoidal_params() const
{
    return sinusoidal_params;
}

const Terrain::PerlinParams& Terrain::get_perlin_params() const
{
    return perlin_params;
}

const Terrain::SimplexParams& Terrain::get_simplex_params() const
{
    return simplex_params;
}

const Terrain::DiamondSquareParams& Terrain::get_diamond_square_params() const
{
    return diamond_square_params;
}

void Terrain::set_sinusoidal_params(const SinusoidalParams& params)
{
    sinusoidal_params = params;
    rebuild_for_active_mode();
}

void Terrain::set_perlin_params(const PerlinParams& params)
{
    perlin_params = params;
    rebuild_for_active_mode();
}

void Terrain::set_simplex_params(const SimplexParams& params)
{
    simplex_params = params;
    rebuild_for_active_mode();
}

void Terrain::set_diamond_square_params(const DiamondSquareParams& params)
{
    diamond_square_params = params;
    rebuild_for_active_mode();
}

void Terrain::rebuild_for_active_mode()
{
    fixed_patch_mesh.destroy();
    diamond_square_mesh.destroy();
    diamond_square_heights.clear();
    diamond_square_size = 0;
    diamond_square_spacing = 1.0f;
    diamond_square_half_span = 0.0f;
    perlin_permutation = make_seeded_permutation(perlin_params.seed);
    simplex_permutation = make_seeded_permutation(simplex_params.seed);

    if (active_mode == Mode::DiamondSquare)
    {
        build_diamond_square_mesh();
        return;
    }

    build_fixed_patch_mesh();
}

void Terrain::append_grid_indices(int grid_cells, std::vector<unsigned int>& indices)
{
    int verts_per_row = grid_cells + 1;

    for (int z = 0; z < grid_cells; ++z)
    {
        for (int x = 0; x < grid_cells; ++x)
        {
            unsigned int i0 = static_cast<unsigned int>(z * verts_per_row + x);
            unsigned int i1 = static_cast<unsigned int>(z * verts_per_row + (x + 1));
            unsigned int i2 = static_cast<unsigned int>((z + 1) * verts_per_row + x);
            unsigned int i3 = static_cast<unsigned int>((z + 1) * verts_per_row + (x + 1));

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }
}

void Terrain::build_vertex_normals(const std::vector<Vec3>& positions,
                                   const std::vector<unsigned int>& indices,
                                   std::vector<Vec3>& normals)
{
    normals.assign(positions.size(), v3(0.0f, 0.0f, 0.0f));

    for (std::size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        unsigned int ia = indices[i + 0];
        unsigned int ib = indices[i + 1];
        unsigned int ic = indices[i + 2];

        Vec3 a = positions[ia];
        Vec3 b = positions[ib];
        Vec3 c = positions[ic];

        Vec3 triangle_normal = cross(sub(b, a), sub(c, a));
        normals[ia] = add(normals[ia], triangle_normal);
        normals[ib] = add(normals[ib], triangle_normal);
        normals[ic] = add(normals[ic], triangle_normal);
    }

    for (std::size_t i = 0; i < normals.size(); ++i)
    {
        Vec3 n = norm(normals[i]);
        if (len(n) == 0.0f)
        {
            n = v3(0.0f, 1.0f, 0.0f);
        }

        normals[i] = n;
    }
}

void Terrain::build_interleaved_vertex_data(const std::vector<Vec3>& positions,
                                            const std::vector<Vec3>& normals,
                                            const std::vector<Vec3>* colors,
                                            std::vector<float>& vertices)
{
    vertices.clear();
    vertices.reserve(positions.size() * 9);

    for (std::size_t i = 0; i < positions.size(); ++i)
    {
        const Vec3& p = positions[i];
        const Vec3& n = normals[i];
        Vec3 c = v3(1.0f, 1.0f, 1.0f);
        if (colors && i < colors->size())
        {
            c = (*colors)[i];
        }
        vertices.push_back(p.x);
        vertices.push_back(p.y);
        vertices.push_back(p.z);
        vertices.push_back(n.x);
        vertices.push_back(n.y);
        vertices.push_back(n.z);
        vertices.push_back(c.x);
        vertices.push_back(c.y);
        vertices.push_back(c.z);
    }
}

void Terrain::build_fixed_patch_mesh()
{
    int verts_per_row = fixed_patch_cells + 1;
    std::size_t vertex_count = static_cast<std::size_t>(verts_per_row * verts_per_row);
    float half_span = static_cast<float>(fixed_patch_cells) * cell_size * 0.5f;

    std::vector<Vec3> positions;
    positions.reserve(vertex_count);

    if (active_mode == Mode::Perlin || active_mode == Mode::Simplex)
    {
        std::vector<float> raw_heights(vertex_count, 0.0f);
        float min_raw = 0.0f;
        float max_raw = 0.0f;
        bool first_value = true;

        for (int z = 0; z <= fixed_patch_cells; ++z)
        {
            for (int x = 0; x <= fixed_patch_cells; ++x)
            {
                float world_x = static_cast<float>(x) * cell_size - half_span;
                float world_z = static_cast<float>(z) * cell_size - half_span;
                float raw = 0.0f;
                if (active_mode == Mode::Perlin)
                {
                    raw = sample_fractal_perlin_2d(world_x,
                                                   world_z,
                                                   perlin_params.base_frequency,
                                                   perlin_params.octaves,
                                                   perlin_params.persistence,
                                                   perlin_params.lacunarity,
                                                   perlin_elevation_offset_x,
                                                   perlin_elevation_offset_z,
                                                   perlin_permutation);
                }
                else
                {
                    raw = sample_fractal_simplex_2d(world_x,
                                                    world_z,
                                                    simplex_params.base_frequency,
                                                    simplex_params.octaves,
                                                    simplex_params.persistence,
                                                    simplex_params.lacunarity,
                                                    simplex_elevation_offset_x,
                                                    simplex_elevation_offset_z,
                                                    simplex_permutation);
                }

                std::size_t index = static_cast<std::size_t>(z * verts_per_row + x);
                raw_heights[index] = raw;

                if (first_value)
                {
                    min_raw = raw;
                    max_raw = raw;
                    first_value = false;
                }
                else
                {
                    min_raw = std::min(min_raw, raw);
                    max_raw = std::max(max_raw, raw);
                }
            }
        }

        float raw_range = max_raw - min_raw;
        for (int z = 0; z <= fixed_patch_cells; ++z)
        {
            for (int x = 0; x <= fixed_patch_cells; ++x)
            {
                std::size_t index = static_cast<std::size_t>(z * verts_per_row + x);
                float normalized = 0.0f;
                if (raw_range > 0.0f)
                {
                    normalized = (raw_heights[index] - min_raw) / raw_range;
                }
                normalized = clampf(normalized, 0.0f, 1.0f);

                if (active_mode == Mode::Perlin)
                {
                    normalized = std::pow(normalized, perlin_params.redistribution_power);
                    normalized *= perlin_params.height_scale;
                }
                else
                {
                    normalized = std::pow(normalized, simplex_params.redistribution_power);
                    normalized *= simplex_params.height_scale;
                }

                float world_x = static_cast<float>(x) * cell_size - half_span;
                float world_z = static_cast<float>(z) * cell_size - half_span;
                positions.push_back(v3(world_x, normalized, world_z));
            }
        }
    }
    else
    {
        for (int z = 0; z <= fixed_patch_cells; ++z)
        {
            for (int x = 0; x <= fixed_patch_cells; ++x)
            {
                float world_x = static_cast<float>(x) * cell_size - half_span;
                float world_z = static_cast<float>(z) * cell_size - half_span;
                float h = sample_height(world_x, world_z);

                positions.push_back(v3(world_x, h, world_z));
            }
        }
    }

    std::vector<unsigned int> indices;
    indices.reserve(static_cast<std::size_t>(fixed_patch_cells * fixed_patch_cells * 6));
    append_grid_indices(fixed_patch_cells, indices);

    std::vector<Vec3> normals;
    build_vertex_normals(positions, indices, normals);

    std::vector<float> vertices;
    build_interleaved_vertex_data(positions, normals, nullptr, vertices);

    fixed_patch_mesh.upload_interleaved_positions_normals_and_indices(vertices, indices);
}

void Terrain::build_diamond_square_mesh()
{
    int ds_exponent = diamond_square_params.exponent;
    int ds_size = (1 << ds_exponent) + 1;
    float ds_initial_amplitude = diamond_square_params.initial_amplitude;
    float ds_roughness = diamond_square_params.roughness;
    float ds_horizontal_spacing = diamond_square_params.spacing;

    std::vector<float> heights(static_cast<std::size_t>(ds_size * ds_size), 0.0f);

    auto at = [&heights, ds_size](int x, int z) -> float& {
        return heights[static_cast<std::size_t>(z * ds_size + x)];
    };

    std::mt19937 rng(static_cast<unsigned int>(diamond_square_params.seed));
    std::uniform_real_distribution<float> corner_dist(-ds_initial_amplitude, ds_initial_amplitude);

    at(0, 0) = corner_dist(rng);
    at(ds_size - 1, 0) = corner_dist(rng);
    at(0, ds_size - 1) = corner_dist(rng);
    at(ds_size - 1, ds_size - 1) = corner_dist(rng);

    int step = ds_size - 1;
    float displacement = ds_initial_amplitude;

    while (step > 1)
    {
        int half = step / 2;
        std::uniform_real_distribution<float> jitter_dist(-displacement, displacement);

        for (int z = half; z < ds_size - 1; z += step)
        {
            for (int x = half; x < ds_size - 1; x += step)
            {
                float a = at(x - half, z - half);
                float b = at(x + half, z - half);
                float c = at(x - half, z + half);
                float d = at(x + half, z + half);

                float avg = (a + b + c + d) * 0.25f;
                at(x, z) = avg + jitter_dist(rng);
            }
        }

        for (int z = 0; z < ds_size; z += half)
        {
            int x_start = ((z / half) % 2 == 0) ? half : 0;
            for (int x = x_start; x < ds_size; x += step)
            {
                float sum = 0.0f;
                int count = 0;

                if (x - half >= 0)
                {
                    sum += at(x - half, z);
                    ++count;
                }
                if (x + half < ds_size)
                {
                    sum += at(x + half, z);
                    ++count;
                }
                if (z - half >= 0)
                {
                    sum += at(x, z - half);
                    ++count;
                }
                if (z + half < ds_size)
                {
                    sum += at(x, z + half);
                    ++count;
                }

                float avg = (count > 0) ? (sum / static_cast<float>(count)) : 0.0f;
                at(x, z) = avg + jitter_dist(rng);
            }
        }

        step = half;
        displacement *= ds_roughness;
    }

    std::vector<Vec3> positions;
    positions.reserve(static_cast<std::size_t>(ds_size * ds_size));

    float half_span = static_cast<float>(ds_size - 1) * ds_horizontal_spacing * 0.5f;
    for (int z = 0; z < ds_size; ++z)
    {
        for (int x = 0; x < ds_size; ++x)
        {
            float world_x = static_cast<float>(x) * ds_horizontal_spacing - half_span;
            float world_z = static_cast<float>(z) * ds_horizontal_spacing - half_span;
            positions.push_back(v3(world_x, at(x, z), world_z));
        }
    }

    std::vector<unsigned int> indices;
    indices.reserve(static_cast<std::size_t>((ds_size - 1) * (ds_size - 1) * 6));
    append_grid_indices(ds_size - 1, indices);

    std::vector<Vec3> normals;
    build_vertex_normals(positions, indices, normals);

    std::vector<float> vertices;
    build_interleaved_vertex_data(positions, normals, nullptr, vertices);

    diamond_square_mesh.upload_interleaved_positions_normals_and_indices(vertices, indices);
    diamond_square_heights = std::move(heights);
    diamond_square_size = ds_size;
    diamond_square_spacing = ds_horizontal_spacing;
    diamond_square_half_span = half_span;
}

void Terrain::update(const Camera& camera)
{
    (void)camera;
}

void Terrain::render(double now, int width, int height, const Camera& camera, bool wireframe_enabled)
{
    (void)now;

    float p[16];
    float v[16];
    float m[16];
    float pv[16];
    float mvp[16];

    float aspect = (height == 0) ? 1.0f : static_cast<float>(width) / static_cast<float>(height);
    mat4_perspective(p, 70.0f * (3.14159265f / 180.0f), aspect, 0.1f, 1000.0f);

    Vec3 cam_pos = camera.get_position();
    Vec3 center = add(cam_pos, camera.get_forward());
    mat4_look_at(v, cam_pos, center, v3(0.0f, 1.0f, 0.0f));

    mat4_identity(m);
    mat4_mul(pv, p, v);
    mat4_mul(mvp, pv, m);

    shader.use();
    shader.set_mat4("mvp", mvp);
    shader.set_vec3("light_dir", 0.4f, 1.0f, 0.35f);
    shader.set_int("color_scheme", static_cast<int>(active_color_scheme));

    if (wireframe_enabled)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (active_mode == Mode::Sinusoidal || active_mode == Mode::Perlin || active_mode == Mode::Simplex)
    {
        fixed_patch_mesh.draw();
    }
    else
    {
        diamond_square_mesh.draw();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Terrain::shutdown()
{
    fixed_patch_mesh.destroy();
    diamond_square_mesh.destroy();
    shader.destroy();
}
float Terrain::sample_fractal_simplex_2d(float world_x,
                                         float world_z,
                                         float base_frequency,
                                         int octaves,
                                         float persistence,
                                         float lacunarity,
                                         float offset_x,
                                         float offset_z,
                                         const std::array<int, 512>& permutation)
{
    float frequency = base_frequency;
    float amplitude = 1.0f;
    float total = 0.0f;
    float amplitude_sum = 0.0f;

    for (int octave = 0; octave < octaves; ++octave)
    {
        float octave_offset_x = offset_x + static_cast<float>(octave) * 53.0f;
        float octave_offset_z = offset_z + static_cast<float>(octave) * 97.0f;
        float sample_x = world_x * frequency + octave_offset_x;
        float sample_z = world_z * frequency + octave_offset_z;
        float noise_sample = simplex_noise_2d(sample_x, sample_z, permutation);
        total += noise_sample * amplitude;
        amplitude_sum += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    if (amplitude_sum <= 0.0f)
    {
        return 0.0f;
    }

    return clampf(total / amplitude_sum, -1.0f, 1.0f);
}

float Terrain::sample_simplex_elevation01(float world_x, float world_z) const
{
    float elevation_raw = sample_fractal_simplex_2d(world_x,
                                                    world_z,
                                                    simplex_params.base_frequency,
                                                    simplex_params.octaves,
                                                    simplex_params.persistence,
                                                    simplex_params.lacunarity,
                                                    simplex_elevation_offset_x,
                                                    simplex_elevation_offset_z,
                                                    simplex_permutation);
    return clampf(elevation_raw * 0.5f + 0.5f, 0.0f, 1.0f);
}

float Terrain::sample_simplex_height(float world_x, float world_z) const
{
    float elevation01 = sample_simplex_elevation01(world_x, world_z);
    return elevation01 * simplex_params.height_scale;
}
