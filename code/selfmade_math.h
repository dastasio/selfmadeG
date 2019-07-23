/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Davide Stasio $
   $Notice: (C) Copyright 2019 by Dave Stasio. All Rights Reserved. $
   ======================================================================== */
#include <cmath>

inline real32
Sin32(real32 a)
{
    real32 Result = sinf(a);
    return(Result);
};

inline real32
Cos32(real32 a)
{
    real32 Result = cosf(a);
    return(Result);
};

inline real32
Tan32(real32 a)
{
    real32 Result = tanf(a);
    return(Result);
};

inline real32
SquareRoot32(real32 Value)
{
    real32 Result = sqrtf(Value);
    return(Result);
};

inline real32
Square(real32 Value)
{
    real32 Result = Value*Value;
    return(Result);
}

inline real32
Absolute(real32 Value)
{
    real32 Result = (Value < 0) ? -Value : Value;
    return(Result);
}

inline real32
Min(real32 A, real32 B)
{
    real32 Result = (A < B) ? A : B;
    return(Result);
}

inline real32
Max(real32 A, real32 B)
{
    real32 Result = (A > B) ? A : B;
    return(Result);
}

inline uint32
Max(uint32 A, uint32 B)
{
    uint32 Result = (A > B) ? A : B;
    return(Result);
}

inline int32
Max(int32 A, int32 B)
{
    int32 Result = (A > B) ? A : B;
    return(Result);
}

union V2
{
    struct
    {
        real32 X;
        real32 Y;
    };
    real32 E[2];
};

union V3
{
    struct 
    {
        real32 X;
        real32 Y;
        real32 Z;
    };
    real32 E[3];

    inline operator bool()
    {
        bool Result = X || Y || Z;
        return(Result);
    }
};

union V4
{
    struct
    {
        real32 X;
        real32 Y;
        real32 Z;
        real32 W;
    };
    real32 E[4];
};

inline V3
operator-(V3 A)
{
    V3 Result;
    Result.X = -A.X;
    Result.Y = -A.Y;
    Result.Z = -A.Z;
    return(Result);
}

inline V3
operator-(V3 A, V3 B)
{
    V3 Result;
    Result.X = A.X - B.X;
    Result.Y = A.Y - B.Y;
    Result.Z = A.Z - B.Z;
    return(Result);
}

inline V3
operator*(V3 A, real32 B)
{
    V3 Result;
    Result.X = A.X*B;
    Result.Y = A.Y*B;
    Result.Z = A.Z*B;
    return(Result);
}

inline V3
operator*(real32 B, V3 A)
{
    V3 Result = A * B;
    return(Result);
}

inline V3
operator+(V3 A, V3 B)
{
    V3 Result;
    Result.X = A.X+B.X;
    Result.Y = A.Y+B.Y;
    Result.Z = A.Z+B.Z;
    return(Result);
}

inline V3
operator/(V3 A, real32 B)
{
    V3 Result;
    Result.X = A.X/B;
    Result.Y = A.Y/B;
    Result.Z = A.Z/B;
    return(Result);
}

inline V3 &
operator+=(V3 &A, V3 B)
{
    A = A + B;
    return(A);
}

inline V3 &
operator-=(V3 &A, V3 B)
{
    A = A - B;
    return(A);
}

inline V3 &
operator/=(V3 &A, real32 B)
{
    A = A / B;
    return(A);
}

inline bool
operator!(V3 A)
{
    bool Result = !(A.X || A.Y || A.Z);
    return(Result);
}

inline bool
operator==(V3 A, V3 B)
{
    bool Result = (A.X == B.X) && (A.Y == B.Y) && (A.Z == B.Z);
    return(Result);
}

inline bool
operator<=(V3 A, V3 B)
{
    bool Result = (A.X <= B.X) && (A.Y <= B.Y) && (A.Z <= B.Z);
    return(Result);
}

inline V3
Absolute(V3 Vector)
{
    V3 Result = Vector;
    if (Result.X < 0) Result.X = -Result.X;
    if (Result.Y < 0) Result.Y = -Result.Y;
    if (Result.Z < 0) Result.Z = -Result.Z;
    return(Result);
}

inline V3
Cross(V3 A, V3 B)
{
    V3 Result;
    Result.X = A.Y*B.Z - A.Z*B.Y;
    Result.Y = A.Z*B.X - A.X*B.Z;
    Result.Z = A.X*B.Y - A.Y*B.X;
    return(Result);
}

inline real32
Inner(V3 A, V3 B)
{
    real32 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z;
    return(Result);
}

inline real32
Inner(V4 A, V4 B)
{
    real32 Result = A.X*B.X + A.Y*B.Y + A.Z*B.Z + A.W*B.W;
    return(Result);
}

inline V3
Reflect(V3 I, V3 N)
{
    V3 Result = I - 2*Inner(N, I)*N;
    return(Result);
}

inline real32
LengthSq(V3 Vector)
{
    real32 Result = Inner(Vector, Vector);
    return(Result);
}

inline real32
Length(V3 Vector)
{
    real32 Result = SquareRoot32(LengthSq(Vector));
    return(Result);
};

inline V3
SquareRoot32(V3 Value)
{
    V3 Result;
    Result.X = SquareRoot32(Value.X);
    Result.Y = SquareRoot32(Value.Y);
    Result.Z = SquareRoot32(Value.Z);
    return(Result);
};

inline V3
Square(V3 Value)
{
    V3 Result;
    Result.X = Value.X*Value.X;
    Result.Y = Value.Y*Value.Y;
    Result.Z = Value.Z*Value.Z;
    return(Result);
}

inline V3
Normalize(V3 Vector)
{
    V3 Result = Vector;
    real32 Modulus = LengthSq(Vector);
    if(Modulus != 1.f)
    {
        Assert(Modulus >= 0.f);
        if(Modulus == 0.f)
        {
            Result = {};
        }
        else
        {
            Result /= SquareRoot32(Modulus);
        }
    }
    return(Result);
};

inline bool
IsUnitVector(V3 Vector)
{
    bool Result = false;
    real32 Modulus = LengthSq(Vector);
    if(Modulus == 1.f)
    {
        Result = true;
    }
    return(Result);
};

union M3
{
    struct
    {
        V3 C0;
        V3 C1;
        V3 C2;
    };
    V3 C[3];
    real32 E[3][3];
};

union M4
{
    struct
    {
        V4 C0;
        V4 C1;
        V4 C2;
        V4 C3;
    };
    V4 C[4];
    real32 E[4][4];
};

inline M3
Transpose(M3 A)
{
    M3 Result;
    Result.E[0][0] = A.E[0][0];
    Result.E[0][1] = A.E[1][0];
    Result.E[0][2] = A.E[2][0];

    Result.E[1][0] = A.E[0][1];
    Result.E[1][1] = A.E[1][1];
    Result.E[1][2] = A.E[2][1];

    Result.E[2][0] = A.E[0][2];
    Result.E[2][1] = A.E[1][2];
    Result.E[2][2] = A.E[2][2];
    return(Result);
};

inline M4
Transpose(M4 A)
{
    M4 Result;
    Result.E[0][0] = A.E[0][0];
    Result.E[0][1] = A.E[1][0];
    Result.E[0][2] = A.E[2][0];
    Result.E[0][3] = A.E[3][0];

    Result.E[1][0] = A.E[0][1];
    Result.E[1][1] = A.E[1][1];
    Result.E[1][2] = A.E[2][1];
    Result.E[1][3] = A.E[3][1];

    Result.E[2][0] = A.E[0][2];
    Result.E[2][1] = A.E[1][2];
    Result.E[2][2] = A.E[2][2];
    Result.E[2][3] = A.E[3][2];

    Result.E[3][0] = A.E[0][3];
    Result.E[3][1] = A.E[1][3];
    Result.E[3][2] = A.E[2][3];
    Result.E[3][3] = A.E[3][3];
    return(Result);
};

inline M4
operator*(M4 A, M4 B)
{
    A = Transpose(A);
    M4 Result;
    for(uint32 Column = 0;
        Column < 4;
        ++Column)
    {
        for(uint32 Row = 0;
            Row < 4;
            ++Row)
        {
            //V4 RowVector = {A.E[Row][0], A.E[Row][1], A.E[Row][2], A.E[Row][3]};
            //V4 ColumnVector = {B.E[Column][0], B.E[Column][1], B.E[Column][2], B.E[Column][3]};
            Result.E[Column][Row] = Inner(A.C[Row], B.C[Column]);
        }
    }
    return(Result);
}

inline M4 &
operator*=(M4 &A, M4 B)
{
    A = A * B;
    return(A);
}

struct camera_space
{
    V3 U;
    V3 V;
    V3 N;

    M4 Matrix;
};

inline M3
IdentityMatrix3()
{
    M3 Result = {};
    Result.E[0][0] = 1.f;
    Result.E[1][1] = 1.f;
    Result.E[2][2] = 1.f;
    return(Result);
};

inline M4
IdentityMatrix4()
{
    M4 Result = {};
    Result.E[0][0] = 1.f;
    Result.E[1][1] = 1.f;
    Result.E[2][2] = 1.f;
    Result.E[3][3] = 1.f;
    return(Result);
};

M4
Matrix3ToMatrix4(M3 A)
{
    M4 Result;
    Result.C[0] = {A.E[0][0], A.E[0][1], A.E[0][2], 0.f}; // Column 0
    Result.C[1] = {A.E[1][0], A.E[1][1], A.E[1][2], 0.f}; // Column 1
    Result.C[2] = {A.E[2][0], A.E[2][1], A.E[2][2], 0.f}; // Column 2
    Result.C[3] = {0.f, 0.f, 0.f, 1.f}; // Column 3
    return(Result);
}

M3
RotationMatrix3(V3 Axis, real32 Angle)
{
    real32 C = Cos32(Angle);
    real32 S = Sin32(Angle);
    real32 OneMinusCos = 1.f - C;
    M3 Result;
    Result.C[0] = { // Column 0
        C + Axis.X*Axis.X*OneMinusCos,
        Axis.Y*Axis.X*OneMinusCos + Axis.Z*S,
        Axis.Z*Axis.X*OneMinusCos - Axis.Y*S
    };
    Result.C[1] = { // Column 1
        Axis.X*Axis.Y*OneMinusCos - Axis.Z*S,
        C + Axis.Y*Axis.Y*OneMinusCos,
        Axis.Z*Axis.Y*OneMinusCos + Axis.X*S
    };
    Result.C[2] = { // Column 2
        Axis.X*Axis.Z*OneMinusCos + Axis.Y*S,
        Axis.Y*Axis.Z*OneMinusCos - Axis.X*S,
        C + Axis.Z*Axis.Z*OneMinusCos
    };
    return(Result);
}

M3
ScaleMatrix3(V3 A)
{
    M3 Result = IdentityMatrix3();
    Result.E[0][0] = A.X;
    Result.E[1][1] = A.Y;
    Result.E[2][2] = A.Z;
    return(Result);
}

M4
RotationMatrix4(V3 Axis, real32 Angle)
{
    M4 Result;
    M3 Rotation3 = RotationMatrix3(Axis, Angle);
    Result = Matrix3ToMatrix4(Rotation3);
    return(Result);
}

M4
ScaleMatrix4(V4 A)
{
    M4 Result = IdentityMatrix4();
    Result.E[0][0] = A.X;
    Result.E[1][1] = A.Y;
    Result.E[2][2] = A.Z;
    Result.E[3][3] = A.W;
    return(Result);
}

M4
TranslationMatrix4(V3 A)
{
    M4 Result = IdentityMatrix4();
    Result.C[3].X = A.X;
    Result.C[3].Y = A.Y;
    Result.C[3].Z = A.Z;
    return(Result);
}

inline M4
PerspectiveProjection(real32 FOV, real32 AR, real32 NearZ, real32 FarZ)
{
    M4 Result = {};
    Result.E[0][0] = 1.f / (AR*Tan32(FOV/2.f));
    Result.E[1][1] = 1.f / Tan32(FOV/2.f);
    Result.E[2][2] = (-NearZ-FarZ) / (NearZ-FarZ);
    Result.E[2][3] = 1.f;
    Result.E[3][2] = (2.f*FarZ*NearZ) / (NearZ-FarZ);
    return(Result);
}

inline M3
NewCoordinateSpace(V3 U, V3 V, V3 N)
{
    M3 Result;
    Result.C[0] = {V.X, U.X, N.X};
    Result.C[1] = {V.Y, U.Y, N.Y};
    Result.C[2] = {V.Z, U.Z, N.Z};
    return(Result);
}

inline M4
ComputeCameraSpace(camera_space *CameraSpace,
                   V3 NewPosition, V3 NewTarget, V3 NewUp)
{
    CameraSpace->N = Normalize(NewTarget - NewPosition);
    CameraSpace->V = Normalize(Cross(CameraSpace->N, NewUp));
    CameraSpace->U = Normalize(Cross(CameraSpace->V, CameraSpace->N));

    V3 &U = CameraSpace->U;
    V3 &V = CameraSpace->V;
    V3 &N = CameraSpace->N;
    M4 Result;
    Result.C[0] = { V.X, U.X, N.X, 0.f };
    Result.C[1] = { V.Y, U.Y, N.Y, 0.f };
    Result.C[2] = { V.Z, U.Z, N.Z, 0.f };
    Result.C[3] = { 0.f, 0.f, 0.f, 1.f };

    Result *= TranslationMatrix4(-NewPosition);
    return(Result);
}
