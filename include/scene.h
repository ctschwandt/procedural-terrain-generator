#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "terrain.h"

class Scene
{
public:
    bool init();
    void update(const Camera& camera);
    void render(double now, int width, int height, const Camera& camera, bool wireframe_enabled);
    void shutdown();

    void set_terrain_mode(Terrain::Mode mode);
    const Terrain& get_terrain() const;
    Terrain& get_terrain_mut();

private:
    Terrain terrain;
};

#endif
