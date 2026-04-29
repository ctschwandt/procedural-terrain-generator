#include "camera.h"

#include <cmath>

#include "terrain.h"

namespace
{
static constexpr float walk_speed = 5.0f;
static constexpr float walk_gravity = -24.0f;
static constexpr float walk_jump_speed = 9.0f;
static constexpr float walk_eye_height = 1.7f;
static constexpr float walk_player_radius = 0.35f;
static constexpr float walk_ground_snap_offset = 0.08f;
}

Camera::Camera()
    : yaw_deg(-90.0f)
    , pitch_deg(0.0f)
    , movement_mode(MovementMode::Fly)
    , mouse_sensitivity(0.07f)
    , cam_pos(v3(0.0f, 1.5f, 6.0f))
    , cam_vel(v3(0.0f, 0.0f, 0.0f))
    , walk_body_pos(v3(0.0f, 1.5f - walk_eye_height, 6.0f))
    , walk_vertical_velocity(0.0f)
    , walk_grounded(false)
    , world_up(v3(0.0f, 1.0f, 0.0f))
    , base_speed(6.0f)
    , fast_mult(3.0f)
    , accel(35.0f)
    , damping(10.0f)
{
}

Vec3 Camera::get_forward() const
{
    const float deg2rad = 3.14159265f / 180.0f;
    float y = yaw_deg * deg2rad;
    float p = pitch_deg * deg2rad;

    Vec3 f;
    f.x = std::cos(y) * std::cos(p);
    f.y = std::sin(p);
    f.z = std::sin(y) * std::cos(p);
    return norm(f);
}

void Camera::update(const Input& input, float dt, const Terrain& terrain, bool allow_mouse_look)
{
    if (allow_mouse_look)
    {
        yaw_deg += static_cast<float>(input.get_mouse_dx()) * mouse_sensitivity;
        pitch_deg -= static_cast<float>(input.get_mouse_dy()) * mouse_sensitivity;
    }

    if (pitch_deg > 89.0f)
    {
        pitch_deg = 89.0f;
    }
    if (pitch_deg < -89.0f)
    {
        pitch_deg = -89.0f;
    }

    if (movement_mode == MovementMode::Fly)
    {
        Vec3 fwd = get_forward();
        Vec3 right = norm(cross(fwd, world_up));

        Vec3 wish = v3(0.0f, 0.0f, 0.0f);
        if (input.is_key_down(GLFW_KEY_W))
        {
            wish = add(wish, fwd);
        }
        if (input.is_key_down(GLFW_KEY_S))
        {
            wish = sub(wish, fwd);
        }
        if (input.is_key_down(GLFW_KEY_D))
        {
            wish = add(wish, right);
        }
        if (input.is_key_down(GLFW_KEY_A))
        {
            wish = sub(wish, right);
        }
        if (input.is_key_down(GLFW_KEY_SPACE))
        {
            wish.y += 1.0f;
        }
        if (input.is_key_down(GLFW_KEY_LEFT_CONTROL))
        {
            wish.y -= 1.0f;
        }

        float wish_len = len(wish);
        if (wish_len > 0.0f)
        {
            wish = mul(wish, 1.0f / wish_len);
        }

        float speed = base_speed;
        if (input.is_key_down(GLFW_KEY_LEFT_SHIFT))
        {
            speed *= fast_mult;
        }

        Vec3 desired_vel = mul(wish, speed);

        if (wish_len > 0.0f)
        {
            Vec3 delta = sub(desired_vel, cam_vel);
            cam_vel = add(cam_vel, mul(delta, accel * dt));
        }
        else
        {
            float k = std::exp(-damping * dt);
            cam_vel = mul(cam_vel, k);
        }

        cam_pos = add(cam_pos, mul(cam_vel, dt));
        return;
    }

    const float deg2rad = 3.14159265f / 180.0f;
    float yaw_rad = yaw_deg * deg2rad;
    Vec3 flat_fwd = v3(std::cos(yaw_rad), 0.0f, std::sin(yaw_rad));
    flat_fwd = norm(flat_fwd);
    Vec3 flat_right = norm(cross(flat_fwd, world_up));

    Vec3 wish = v3(0.0f, 0.0f, 0.0f);
    if (input.is_key_down(GLFW_KEY_W))
    {
        wish = add(wish, flat_fwd);
    }
    if (input.is_key_down(GLFW_KEY_S))
    {
        wish = sub(wish, flat_fwd);
    }
    if (input.is_key_down(GLFW_KEY_D))
    {
        wish = add(wish, flat_right);
    }
    if (input.is_key_down(GLFW_KEY_A))
    {
        wish = sub(wish, flat_right);
    }

    float horizontal_len = len(wish);
    if (horizontal_len > 0.0f)
    {
        wish = mul(wish, 1.0f / horizontal_len);
    }

    float horizontal_speed = walk_speed;
    if (input.is_key_down(GLFW_KEY_LEFT_SHIFT))
    {
        horizontal_speed *= fast_mult;
    }
    Vec3 horizontal_delta = mul(wish, horizontal_speed * dt);
    walk_body_pos.x += horizontal_delta.x;
    walk_body_pos.z += horizontal_delta.z;

    if (walk_grounded && input.is_key_pressed(GLFW_KEY_SPACE))
    {
        walk_vertical_velocity = walk_jump_speed;
        walk_grounded = false;
    }

    if (!walk_grounded || walk_vertical_velocity > 0.0f)
    {
        walk_vertical_velocity += walk_gravity * dt;
        walk_body_pos.y += walk_vertical_velocity * dt;
    }
    else
    {
        walk_vertical_velocity = 0.0f;
    }

    float ground_height_center = terrain.get_height_at_world(walk_body_pos.x, walk_body_pos.z);
    float ground_height_pos_x = terrain.get_height_at_world(walk_body_pos.x + walk_player_radius, walk_body_pos.z);
    float ground_height_neg_x = terrain.get_height_at_world(walk_body_pos.x - walk_player_radius, walk_body_pos.z);
    float ground_height_pos_z = terrain.get_height_at_world(walk_body_pos.x, walk_body_pos.z + walk_player_radius);
    float ground_height_neg_z = terrain.get_height_at_world(walk_body_pos.x, walk_body_pos.z - walk_player_radius);
    float ground_height = std::max(ground_height_center, ground_height_pos_x);
    ground_height = std::max(ground_height, ground_height_neg_x);
    ground_height = std::max(ground_height, ground_height_pos_z);
    ground_height = std::max(ground_height, ground_height_neg_z);

    float body_above_ground = walk_body_pos.y - ground_height;
    bool is_descending_or_stationary = walk_vertical_velocity <= 0.0f;
    bool is_penetrating_ground = body_above_ground < 0.0f;
    bool is_within_snap_window = body_above_ground <= walk_ground_snap_offset;
    if (is_penetrating_ground)
    {
        walk_body_pos.y = ground_height;
        if (walk_vertical_velocity < 0.0f)
        {
            walk_vertical_velocity = 0.0f;
        }
        walk_grounded = true;
    }
    else if (is_descending_or_stationary && is_within_snap_window)
    {
        walk_body_pos.y = ground_height;
        walk_vertical_velocity = 0.0f;
        walk_grounded = true;
    }
    else
    {
        walk_grounded = false;
    }

    cam_pos = add(walk_body_pos, v3(0.0f, walk_eye_height, 0.0f));
    cam_vel = v3(0.0f, 0.0f, 0.0f);
}

Vec3 Camera::get_position() const
{
    return cam_pos;
}

float Camera::get_yaw_deg() const
{
    return yaw_deg;
}

float Camera::get_pitch_deg() const
{
    return pitch_deg;
}

void Camera::toggle_movement_mode()
{
    if (movement_mode == MovementMode::Fly)
    {
        movement_mode = MovementMode::Walk;
        walk_body_pos = sub(cam_pos, v3(0.0f, walk_eye_height, 0.0f));
        walk_vertical_velocity = 0.0f;
        walk_grounded = false;
    }
    else
    {
        movement_mode = MovementMode::Fly;
        cam_pos = add(walk_body_pos, v3(0.0f, walk_eye_height, 0.0f));
        cam_vel = v3(0.0f, 0.0f, 0.0f);
    }
}

Camera::MovementMode Camera::get_movement_mode() const
{
    return movement_mode;
}
