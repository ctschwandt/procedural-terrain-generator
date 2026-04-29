#include "heightmap.h"

Heightmap::Heightmap()
    : width(0)
    , depth(0)
{
}

void Heightmap::resize(int new_width, int new_depth)
{
    width = new_width;
    depth = new_depth;
    values.assign(static_cast<std::size_t>(width * depth), 0.0f);
}

float& Heightmap::at(int x, int z)
{
    return values[static_cast<std::size_t>(z * width + x)];
}

float Heightmap::at(int x, int z) const
{
    return values[static_cast<std::size_t>(z * width + x)];
}

int Heightmap::get_width() const
{
    return width;
}

int Heightmap::get_depth() const
{
    return depth;
}
