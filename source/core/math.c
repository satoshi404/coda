#include "math.h"
#include <string.h> 
#include <math.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define __namespace(func_name) core_Math##func_name

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Mat4 __namespace(Mat4Multiply)(Mat4 a, Mat4 b)
{
    Mat4 result;
    memset(result.m, 0, sizeof(result.m));

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i * 4 + j] =
                a.m[i * 4 + 0] * b.m[0 * 4 + j] +
                a.m[i * 4 + 1] * b.m[1 * 4 + j] +
                a.m[i * 4 + 2] * b.m[2 * 4 + j] +
                a.m[i * 4 + 3] * b.m[3 * 4 + j];
        }
    }
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Mat4 __namespace(Mat4Translate)(Vec3 translation)
{
    Mat4 mat = __namespace(Mat4Identity)();
    mat.m[12] = translation.x;
    mat.m[13] = translation.y;
    mat.m[14] = translation.z;
    return mat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Mat4 __namespace(Mat4Scale)(Vec3 scale)
{
    Mat4 mat = __namespace(Mat4Identity)();
    mat.m[0]  = scale.x;
    mat.m[5]  = scale.y;
    mat.m[10] = scale.z;
    return mat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Mat4 __namespace(Mat4Rotate)(f32 angle, Vec3 axis)
{
    Mat4 mat = __namespace(Mat4Identity)();

    f32 c = cosf(angle);
    f32 s = sinf(angle);
    f32 t = 1.0f - c;

    Vec3 n = __namespace(Vec3Normalize)(axis);
    if (__namespace(Vec3Length)(n) == 0.0f) {
        return mat; // eixo inválido
    }

    f32 x = n.x, y = n.y, z = n.z;

    mat.m[0]  = t * x * x + c;
    mat.m[1]  = t * x * y + s * z;
    mat.m[2]  = t * x * z - s * y;

    mat.m[4]  = t * x * y - s * z;
    mat.m[5]  = t * y * y + c;
    mat.m[6]  = t * y * z + s * x;

    mat.m[8]  = t * x * z + s * y;
    mat.m[9]  = t * y * z - s * x;
    mat.m[10] = t * z * z + c;

    return mat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Mat4 __namespace(Mat4Perspective)(f32 fov, f32 aspect, f32 near, f32 far)
{
    Mat4 mat;
    memset(mat.m, 0, sizeof(mat.m));

    f32 f = 1.0f / tanf(fov * 0.5f);

    mat.m[0]  = f / aspect;
    mat.m[5]  = f;
    mat.m[10] = (far + near) / (near - far);
    mat.m[11] = -1.0f;
    mat.m[14] = (2.0f * far * near) / (near - far);
    mat.m[15] = 0.0f;

    return mat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Mat4 __namespace(Mat4LookAt)(Vec3 eye, Vec3 center, Vec3 up)
{
    Vec3 f = __namespace(Vec3Normalize)(__namespace(Vec3Sub)(center, eye));
    Vec3 s = __namespace(Vec3Normalize)(__namespace(Vec3Cross)(f, up));
    Vec3 u = __namespace(Vec3Cross)(s, f);

    Mat4 mat = __namespace(Mat4Identity)();
    mat.m[0]  = s.x;
    mat.m[1]  = u.x;
    mat.m[2]  = -f.x;

    mat.m[4]  = s.y;
    mat.m[5]  = u.y;
    mat.m[6]  = -f.y;

    mat.m[8]  = s.z;
    mat.m[9]  = u.z;
    mat.m[10] = -f.z;

    mat.m[12] = -__namespace(Vec3Dot)(s, eye);
    mat.m[13] = -__namespace(Vec3Dot)(u, eye);
    mat.m[14] =  __namespace(Vec3Dot)(f, eye);

    return mat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Mat4 __namespace( TransformToMatrix ) ( const Transform* transform )
{
    Mat4 translate = __namespace(Mat4Translate)(transform->position);
    Mat4 scale     = __namespace(Mat4Scale)(transform->scale);

    Mat4 rotX = __namespace(Mat4Rotate)(transform->rotation.x, __namespace(Vec3Create)(1, 0, 0));
    Mat4 rotY = __namespace(Mat4Rotate)(transform->rotation.y, __namespace(Vec3Create)(0, 1, 0));
    Mat4 rotZ = __namespace(Mat4Rotate)(transform->rotation.z, __namespace(Vec3Create)(0, 0, 1));
    Mat4 rotation = __namespace(Mat4Multiply)(rotZ, __namespace(Mat4Multiply)(rotY, rotX));

    Mat4 model = __namespace(Mat4Multiply)(translate, __namespace(Mat4Multiply)(rotation, scale));
    return model;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef __namespace

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
