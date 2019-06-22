/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */
#include <cmath>

// TODO(dave): Remove this forword declarations
struct mat4;
mat4 Multiply(mat4 a, mat4 b);

real32 Sin32(real32 a)
{
    real32 Result = sinf(a);
    return Result;
};

real32 Cos32(real32 a)
{
    real32 Result = cosf(a);
    return Result;
};

real32 Tan32(real32 a)
{
    real32 Result = tanf(a);
    return Result;
};

real32 SquareRoot32(real32 Value)
{
    real32 Result = sqrtf(Value);
    return Result;
};

typedef struct vec2
{
    union
    {
        struct
        {
            GLfloat x;
            GLfloat y;
        };
        //struct
        //{
        //    GLfloat r;
        //    GLfloat g;
        //};
    };
} vec2;

typedef struct vec3
{
    union
    {
        struct 
        {
            GLfloat x;
            GLfloat y;
            GLfloat z;
        };
        //struct
        //{
        //    GLfloat r;
        //    GLfloat g;
        //    GLfloat b;
        //};
    };

    vec3 operator-()
    {
        vec3 Result = {-x, -y, -z};
        return Result;
    }

    vec3 operator-(vec3 b)
    {
        vec3 Result = {x - b.x, y - b.y, z - b.z};
        return Result;
    }

    vec3 operator*(real32 m)
    {
        vec3 Result = {x*m, y*m, z*m};
        return Result;
    }

    vec3 operator+(vec3 b)
    {
        vec3 Result = {x+b.x, y+b.y, z+b.z};
        return Result;
    }

    vec3 operator/(real32 d)
    {
        vec3 Result = {x, y, z};
        Result.x /= d;
        Result.y /= d;
        Result.z /= d;
        return Result;
    }

    void operator+=(vec3 b)
    {
        x += b.x;
        y += b.y;
        z += b.z;
    }

    void operator-=(vec3 b)
    {
        x -= b.x;
        y -= b.y;
        z -= b.z;
    }

    void operator/=(real32 d)
    {
        x /= d;
        y /= d;
        z /= d;
    }
    
    GLfloat &operator[](uint32 i)
    {
        return *((GLfloat *)this + i);
    }
} vec3;
vec3 operator*(real32 a, vec3 b)
{
    vec3 Result = {a*b.x, a*b.y, a*b.z};
    return Result;
}

typedef struct vec4
{
    union
    {
        struct
        {
            GLfloat x;
            GLfloat y;
            GLfloat z;
            GLfloat w;
        };
        //struct
        //{
        //    GLfloat r;
        //    GLfloat g;
        //    GLfloat b;
        //    GLfloat a;
        //};
    };

    GLfloat &operator[](uint32 i)
    {
        return *((GLfloat *)this + i);
    }
} vec4;

typedef struct mat3
{
    vec3 c0;
    vec3 c1;
    vec3 c2;

    vec3 &operator[](uint32 i)
    {
        return *((vec3 *)this + i);
    }
} mat3;

typedef struct mat4
{
    vec4 c0;
    vec4 c1;
    vec4 c2;
    vec4 c3;
    
    mat4 operator*(mat4 b)
    {
        return Multiply(*this, b);
    }

    void operator*=(mat4 b)
    {
        *this = Multiply(*this, b);
    }

    vec4 &operator[](uint32 i)
    {
        return *((vec4 *)this + i);
    }
} mat4;

struct camera_space
{
    vec3 U;
    vec3 V;
    vec3 N;

    mat4 Matrix;
};

real32
VectorLength(vec3 Vector)
{
    real32 Result = SquareRoot32(
        Vector.x*Vector.x +
        Vector.y*Vector.y +
        Vector.z*Vector.z
    );
    return Result;
};

vec3
Normalize(vec3 Vector)
{
    vec3 Result = Vector;
    real32 Length = VectorLength(Vector);
    if(Length != 1.f)
    {
        Result /= Length;
    }
    return Result;
};

vec3
Absolute(vec3 Vector)
{
    vec3 Result = Vector;
    if (Result.x < 0) Result.x = -Result.x;
    if (Result.y < 0) Result.y = -Result.y;
    if (Result.z < 0) Result.z = -Result.z;
    return Result;
}

real32
DotProduct(vec3 a, vec3 b)
{
    real32 Result = a.x*b.x + a.y*b.y + a.z*b.z;
    return Result;
}

real32
DotProduct(vec4 a, vec4 b)
{
    real32 Result = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    return Result;
}

vec3
CrossProduct(vec3 a, vec3 b)
{
    vec3 Result = {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
    return Result;
}

bool
IsUnitVector(vec3 Vector)
{
    bool Result = false;
    real32 Length = VectorLength(Vector);
    if(Length == 1.f)
    {
        Result = true;
    }
    return Result;
};

inline mat3
IdentityMatrix3()
{
    mat3 Result = {};
    Result[0][0] = 1.f;
    Result[1][1] = 1.f;
    Result[2][2] = 1.f;

    return Result;
};

inline mat3
Transpose(mat3 m)
{
    mat3 Result = {
        {m[0][0], m[1][0], m[2][0]},
        {m[0][1], m[1][1], m[2][1]},
        {m[0][2], m[1][2], m[2][2]}
    };
    return Result;
};

inline mat4
IdentityMatrix4()
{
    mat4 Result = {};
    Result[0][0] = 1.f;
    Result[1][1] = 1.f;
    Result[2][2] = 1.f;
    Result[3][3] = 1.f;

    return Result;
};

inline mat4
Transpose(mat4 m)
{
    mat4 Result = {
        {m[0][0], m[1][0], m[2][0], m[3][0]},
        {m[0][1], m[1][1], m[2][1], m[3][1]},
        {m[0][2], m[1][2], m[2][2], m[3][2]},
        {m[0][3], m[1][3], m[2][3], m[3][3]}
    };
    return Result;
};

inline mat4
Multiply(mat4 a, mat4 b)
{
    a = Transpose(a);
    mat4 Result;
    for(uint32 Column = 0;
        Column < 4;
        ++Column)
    {
        for(uint32 Row = 0;
            Row < 4;
            ++Row)
        {
            Result[Column][Row] = DotProduct(a[Row], b[Column]);
        }
    }
    return Result;
}

mat4
Matrix3ToMatrix4(mat3 m)
{
    mat4 Result = { 
        {m[0].x, m[0].y, m[0].z, 0.f}, // Column 0
        {m[1].x, m[1].y, m[1].z, 0.f}, // Column 1
        {m[2].x, m[2].y, m[2].z, 0.f}, // Column 2
        {m[3].x, m[3].y, m[3].z, 1.f}  // Column 3
    };
    return Result;
}

mat3
RotationMatrix3(vec3 Axis, real32 Angle)
{
    real32 C = Cos32(Angle);
    real32 S = Sin32(Angle);
    real32 OneMinusCos = 1.f - C;
    mat3 Result =
    {
        { // Column 0
            C + Axis.x*Axis.x*OneMinusCos,
            Axis.y*Axis.x*OneMinusCos + Axis.z*S,
            Axis.z*Axis.x*OneMinusCos - Axis.y*S
        },
        { // Column 1
            Axis.x*Axis.y*OneMinusCos - Axis.z*S,
            C + Axis.y*Axis.y*OneMinusCos,
            Axis.z*Axis.y*OneMinusCos + Axis.x*S
        },
        { // Column 2
            Axis.x*Axis.z*OneMinusCos + Axis.y*S,
            Axis.y*Axis.z*OneMinusCos - Axis.x*S,
            C + Axis.z*Axis.z*OneMinusCos
        }
    };

    return Result;
}

mat3
ScaleMatrix3(vec3 s)
{
    mat3 Result = IdentityMatrix3();
    Result[0][0] = s.x;
    Result[1][1] = s.y;
    Result[2][2] = s.z;
    return Result;
}

mat4
RotationMatrix4(vec3 Axis, real32 Angle)
{
    mat4 Result;
    mat3 Rotation3 = RotationMatrix3(Axis, Angle);
    Result = Matrix3ToMatrix4(Rotation3);
    return Result;
}

mat4
ScaleMatrix4(vec4 s)
{
    mat4 Result = IdentityMatrix4();
    Result[0][0] = s.x;
    Result[1][1] = s.y;
    Result[2][2] = s.z;
    Result[3][3] = s.w;
    return Result;
}

mat4
TranslationMatrix4(vec3 t)
{
    mat4 Result = IdentityMatrix4();
    Result[3].x = t.x;
    Result[3].y = t.y;
    Result[3].z = t.z;

    return Result;
}

inline mat4
PerspectiveProjection(real32 FOV, real32 AR, real32 NearZ, real32 FarZ)
{
    mat4 Result = {};
    Result[0][0] = 1.f / (AR*Tan32(FOV/2.f));
    Result[1][1] = 1.f / Tan32(FOV/2.f);
    Result[2][2] = (-NearZ-FarZ) / (NearZ-FarZ);
    Result[2][3] = 1.f;
    Result[3][2] = (2.f*FarZ*NearZ) / (NearZ-FarZ);
    return Result;
}

inline mat4
ComputeCameraSpace(camera_space *CameraSpace,
                   vec3 NewPosition, vec3 NewTarget, vec3 NewUp)
{
    CameraSpace->N = Normalize(NewTarget - NewPosition);
    CameraSpace->V = Normalize(CrossProduct(CameraSpace->N, NewUp));
    CameraSpace->U = Normalize(CrossProduct(CameraSpace->V, CameraSpace->N));

    vec3 &U = CameraSpace->U;
    vec3 &V = CameraSpace->V;
    vec3 &N = CameraSpace->N;
    mat4 Result = {
        { V.x, U.x, N.x, 0.f },
        { V.y, U.y, N.y, 0.f },
        { V.z, U.z, N.z, 0.f },
        { 0.f, 0.f, 0.f, 1.f }
    };

    Result *= TranslationMatrix4(-NewPosition);
    return Result;
}
