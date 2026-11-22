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

const Terrain& Scene::get_terrain() const
{
    return terrain;
}

Terrain& Scene::get_terrain_mut()
{
    return terrain;
}
