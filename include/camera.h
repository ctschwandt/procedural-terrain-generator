#ifndef CAMERA_H
#define CAMERA_H

#include "input.h"
#include "math.h"

class Terrain;

class Camera
{
public:
    enum class MovementMode
    {
        Fly,
        Walk
    };

    Camera();

    void update(const Input& input, float dt, const Terrain& terrain, bool allow_mouse_look = true);
    void toggle_movement_mode();
    MovementMode get_movement_mode() const;

    Vec3 get_position() const;
    Vec3 get_forward() const;
    float get_yaw_deg() const;
    float get_pitch_deg() const;

private:
    float yaw_deg;
    float pitch_deg;
    MovementMode movement_mode;

    float mouse_sensitivity;

    Vec3 cam_pos;
    Vec3 cam_vel;
    Vec3 walk_body_pos;
    float walk_vertical_velocity;
    bool walk_grounded;
    Vec3 world_up;

    float base_speed;
    float fast_mult;
    float accel;
    float damping;
};

#endif
