#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <vector>

class Heightmap
{
public:
    Heightmap();

    void resize(int width, int depth);

    float& at(int x, int z);
    float at(int x, int z) const;

    int get_width() const;
    int get_depth() const;

private:
    int width;
    int depth;
    std::vector<float> values;
};

#endif
