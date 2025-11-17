#ifndef __math_h__
#define __math_h__

#include <core/types.h>
#include <math.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define __namespace( func_name ) core##_##Math##func_name

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct_name ( Vec2 ) { f32 x, y; };

static inline Vec2 __namespace( Vec2Create ) ( f32 x, f32 y ) { return ( Vec2 ) { x, y };  }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct_name ( Vec3 ) { f32 x, y, z; };


static inline Vec3 __namespace( Vec3Create ) ( f32 x, f32 y, f32 z ) { return (Vec3) { x, y, z }; }

static inline Vec3 __namespace( Vec3Add ) ( Vec3 a, Vec3 b )
{
    return core_MathVec3Create(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vec3 __namespace( Vec3Sub ) ( Vec3 a, Vec3 b )
{
    return core_MathVec3Create( a.x - b.x, a.y - b.y, a.z - b.z );
}

static inline Vec3 __namespace( Vec3Scale ) ( Vec3 v, f32 s )
{
    return core_MathVec3Create( v.x * s, v.y * s, v.z * s );
}

static inline f32 __namespace( Vec3Dot ) ( Vec3 a, Vec3 b )
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 __namespace( Vec3Cross ) ( Vec3 a, Vec3 b )
{
    return core_MathVec3Create(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static inline f32 __namespace( Vec3Length ) ( Vec3 v )
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline Vec3 __namespace( Vec3Normalize ) ( Vec3 v )
{
    f32 len = core_MathVec3Length(v);
    if (len > 0.0f) {
        return core_MathVec3Scale(v, 1.0f / len);
    }
    return v;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct_name ( Vec4 ) { f32 x, y, z, w; };

static inline Vec4 __namespace( Vec4Create ) ( f32 x, f32 y, f32 z, f32 w ) { return ( Vec4 ) { x, y, z, w }; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct_name ( Mat4 ) { f32 m[16]; };

static inline Mat4 __namespace( Mat4Identity ) ( void )
{
    Mat4 mat =
    {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }};
    return mat;
}

Mat4 __namespace( Mat4Multiply )    ( Mat4 a, Mat4 b );
Mat4 __namespace( Mat4Translate )   ( Vec3 translation );
Mat4 __namespace( Mat4Rotate )      ( f32 angle, Vec3 axis );
Mat4 __namespace( Mat4Scale )       ( Vec3 scale );
Mat4 __namespace( Mat4Perspective ) ( f32 fov, f32 aspect, f32 near, f32 far );
Mat4 __namespace( Mat4LookAt )      ( Vec3 eye, Vec3 center, Vec3 up );

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct_name ( Transform )
{
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
};

static inline Transform  __namespace( TransformCreate ) ( void )
{
    Transform t;
    t.position = core_MathVec3Create ( 0, 0, 0 );
    t.rotation = core_MathVec3Create ( 0, 0, 0 );
    t.scale    = core_MathVec3Create ( 1, 1, 1 );
    return t;
}

Mat4 __namespace( TransformToMatrix ) ( const Transform* transform );

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* __math_h__ */