#include "scene.h"

bool Scene::init()
{
    return terrain.init();
}

void Scene::update(const Camera& camera)
{
    terrain.update(camera);
}

void Scene::render(double now, int width, int height, const Camera& camera, bool wireframe_enabled)
{
    terrain.render(now, width, height, camera, wireframe_enabled);
}

void Scene::shutdown()
{
    terrain.shutdown();
}

void Scene::set_terrain_mode(Terrain::Mode mode)
{
    terrain.set_mode(mode);
}

void Scene::set_terrain_color_scheme(Terrain::ColorScheme color_scheme)
{
    terrain.set_color_scheme(color_scheme);
}

const Terrain& Scene::get_terrain() const
{
    return terrain;
}

Terrain& Scene::get_terrain_mut()
{
    return terrain;
}
