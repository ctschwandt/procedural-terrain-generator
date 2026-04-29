#ifndef MATH_H
#define MATH_H

#include <cmath>

struct Vec3
{
    float x;
    float y;
    float z;
};

inline Vec3 v3(float x, float y, float z)
{
    return {x, y, z};
}

inline Vec3 add(Vec3 a, Vec3 b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Vec3 sub(Vec3 a, Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline Vec3 mul(Vec3 a, float s)
{
    return {a.x * s, a.y * s, a.z * s};
}

inline float dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3 cross(Vec3 a, Vec3 b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline float len(Vec3 a)
{
    return std::sqrt(dot(a, a));
}

inline Vec3 norm(Vec3 a)
{
    float l = len(a);
    if (l > 0.0f)
    {
        return mul(a, 1.0f / l);
    }
    return v3(0.0f, 0.0f, 0.0f);
}

inline float clampf(float x, float a, float b)
{
    if (x < a)
    {
        return a;
    }
    if (x > b)
    {
        return b;
    }
    return x;
}

inline void mat4_identity(float m[16])
{
    for (int i = 0; i < 16; i++)
    {
        m[i] = 0.0f;
    }
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

inline void mat4_mul(float out[16], const float a[16], const float b[16])
{
    float r[16];
    for (int col = 0; col < 4; col++)
    {
        for (int row = 0; row < 4; row++)
        {
            r[col * 4 + row] =
                a[0 * 4 + row] * b[col * 4 + 0] +
                a[1 * 4 + row] * b[col * 4 + 1] +
                a[2 * 4 + row] * b[col * 4 + 2] +
                a[3 * 4 + row] * b[col * 4 + 3];
        }
    }

    for (int i = 0; i < 16; i++)
    {
        out[i] = r[i];
    }
}

inline void mat4_perspective(float m[16], float fovy, float aspect, float zn, float zf)
{
    for (int i = 0; i < 16; i++)
    {
        m[i] = 0.0f;
    }

    float f = 1.0f / std::tan(fovy * 0.5f);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (zf + zn) / (zn - zf);
    m[11] = -1.0f;
    m[14] = (2.0f * zf * zn) / (zn - zf);
}

inline void mat4_look_at(float m[16], Vec3 eye, Vec3 center, Vec3 up)
{
    Vec3 f = norm(sub(center, eye));
    Vec3 s = norm(cross(f, up));
    Vec3 u = cross(s, f);

    mat4_identity(m);

    m[0] = s.x;
    m[4] = s.y;
    m[8] = s.z;

    m[1] = u.x;
    m[5] = u.y;
    m[9] = u.z;

    m[2] = -f.x;
    m[6] = -f.y;
    m[10] = -f.z;

    m[12] = -dot(s, eye);
    m[13] = -dot(u, eye);
    m[14] = dot(f, eye);
}

#endif
