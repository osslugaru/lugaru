/*
Copyright (C) 2003, 2010 - Wolfire Games

This file is part of Lugaru.

Lugaru is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _PHYSICSMATH_H_
#define _PHYSICSMATH_H_

//#include <Carbon.h>

#include "MacCompatibility.h"

//------------------------------------------------------------------------//
// Misc. Constants
//------------------------------------------------------------------------//

float const pi = 3.14159265f;
float const g = -32.174f; // acceleration due to gravity, ft/s^2
float const rho = 0.0023769f; // desity of air at sea level, slugs/ft^3
float const tol = 0.0000000001f; // float type tolerance


//------------------------------------------------------------------------//
// Misc. Functions
//------------------------------------------------------------------------//
inline float DegreesToRadians(float deg);
inline float RadiansToDegrees(float rad);

inline float DegreesToRadians(float deg)
{
    return deg * pi / 180.0f;
}

inline float RadiansToDegrees(float rad)
{
    return rad * 180.0f / pi;
}

//------------------------------------------------------------------------//
// Vector Class and vector functions
//------------------------------------------------------------------------//
class Vector
{
public:
    float x;
    float y;
    float z;

    Vector(void);
    Vector(float xi, float yi, float zi);

    float Magnitude(void);
    void  Normalize(void);
    void  Reverse(void);

    Vector& operator+=(Vector u); // vector addition
    Vector& operator-=(Vector u); // vector subtraction
    Vector& operator*=(float s); // scalar multiply
    Vector& operator/=(float s); // scalar divide

    Vector operator-(void);

};

inline Vector operator+(Vector u, Vector v);
inline Vector operator-(Vector u, Vector v);
inline Vector operator^(Vector u, Vector v);
inline float operator*(Vector u, Vector v);
inline Vector operator*(float s, Vector u);
inline Vector operator*(Vector u, float s);
inline Vector operator/(Vector u, float s);
inline float TripleScalarProduct(Vector u, Vector v, Vector w);
/*
float fast_sqrt2 (register float arg);
float fast_sqrt2 (register float arg)
{
// Can replace with slower return std::sqrt(arg);
register float result;

if (arg == 0.0) return 0.0;

asm {
frsqrte     result,arg          // Calculate Square root
}

// Newton Rhapson iterations.
result = result + 0.5 * result * (1.0 - arg * result * result);
result = result + 0.5 * result * (1.0 - arg * result * result);

return result * arg;
}
*/
inline Vector::Vector(void)
{
    x = 0;
    y = 0;
    z = 0;
}

inline Vector::Vector(float xi, float yi, float zi)
{
    x = xi;
    y = yi;
    z = zi;
}

inline float Vector::Magnitude(void)
{
    return (float) sqrt(x * x + y * y + z * z);
}

inline void  Vector::Normalize(void)
{
    float m = (float) sqrt(x * x + y * y + z * z);
    if (m <= tol)
        m = 1;
    x /= m;
    y /= m;
    z /= m;

    if (fabs(x) < tol)
        x = 0.0f;
    if (fabs(y) < tol)
        y = 0.0f;
    if (fabs(z) < tol)
        z = 0.0f;
}

inline void  Vector::Reverse(void)
{
    x = -x;
    y = -y;
    z = -z;
}

inline Vector& Vector::operator+=(Vector u)
{
    x += u.x;
    y += u.y;
    z += u.z;
    return *this;
}

inline Vector& Vector::operator-=(Vector u)
{
    x -= u.x;
    y -= u.y;
    z -= u.z;
    return *this;
}

inline Vector& Vector::operator*=(float s)
{
    x *= s;
    y *= s;
    z *= s;
    return *this;
}

inline Vector& Vector::operator/=(float s)
{
    x /= s;
    y /= s;
    z /= s;
    return *this;
}

inline Vector Vector::operator-(void)
{
    return Vector(-x, -y, -z);
}


inline Vector operator+(Vector u, Vector v)
{
    return Vector(u.x + v.x, u.y + v.y, u.z + v.z);
}

inline Vector operator-(Vector u, Vector v)
{
    return Vector(u.x - v.x, u.y - v.y, u.z - v.z);
}

// Vector cross product (u cross v)
inline Vector operator^(Vector u, Vector v)
{
    return Vector(  u.y * v.z - u.z * v.y,
                    -u.x * v.z + u.z * v.x,
                    u.x * v.y - u.y * v.x );
}

// Vector dot product
inline float operator*(Vector u, Vector v)
{
    return (u.x * v.x + u.y * v.y + u.z * v.z);
}

inline Vector operator*(float s, Vector u)
{
    return Vector(u.x * s, u.y * s, u.z * s);
}

inline Vector operator*(Vector u, float s)
{
    return Vector(u.x * s, u.y * s, u.z * s);
}

inline Vector operator/(Vector u, float s)
{
    return Vector(u.x / s, u.y / s, u.z / s);
}

// triple scalar product (u dot (v cross w))
inline float TripleScalarProduct(Vector u, Vector v, Vector w)
{
    return float(   (u.x * (v.y * w.z - v.z * w.y)) +
                    (u.y * (-v.x * w.z + v.z * w.x)) +
                    (u.z * (v.x * w.y - v.y * w.x)) );
    //return u*(v^w);

}



//------------------------------------------------------------------------//
// Matrix Class and matrix functions
//------------------------------------------------------------------------//

class Matrix3x3
{
public:
    // elements eij: i -> row, j -> column
    float e11, e12, e13, e21, e22, e23, e31, e32, e33;

    Matrix3x3(void);
    Matrix3x3(  float r1c1, float r1c2, float r1c3,
                float r2c1, float r2c2, float r2c3,
                float r3c1, float r3c2, float r3c3 );

    float det(void);
    Matrix3x3 Transpose(void);
    Matrix3x3 Inverse(void);

    Matrix3x3& operator+=(Matrix3x3 m);
    Matrix3x3& operator-=(Matrix3x3 m);
    Matrix3x3& operator*=(float s);
    Matrix3x3& operator/=(float s);
};

inline Matrix3x3 operator+(Matrix3x3 m1, Matrix3x3 m2);
inline Matrix3x3 operator-(Matrix3x3 m1, Matrix3x3 m2);
inline Matrix3x3 operator/(Matrix3x3 m, float s);
inline Matrix3x3 operator*(Matrix3x3 m1, Matrix3x3 m2);
inline Matrix3x3 operator*(Matrix3x3 m, float s);
inline Matrix3x3 operator*(float s, Matrix3x3 m);
inline Vector operator*(Matrix3x3 m, Vector u);
inline Vector operator*(Vector u, Matrix3x3 m);





inline Matrix3x3::Matrix3x3(void)
{
    e11 = 0;
    e12 = 0;
    e13 = 0;
    e21 = 0;
    e22 = 0;
    e23 = 0;
    e31 = 0;
    e32 = 0;
    e33 = 0;
}

inline Matrix3x3::Matrix3x3(    float r1c1, float r1c2, float r1c3,
                                float r2c1, float r2c2, float r2c3,
                                float r3c1, float r3c2, float r3c3 )
{
    e11 = r1c1;
    e12 = r1c2;
    e13 = r1c3;
    e21 = r2c1;
    e22 = r2c2;
    e23 = r2c3;
    e31 = r3c1;
    e32 = r3c2;
    e33 = r3c3;
}

inline float Matrix3x3::det(void)
{
    return  e11 * e22 * e33 -
            e11 * e32 * e23 +
            e21 * e32 * e13 -
            e21 * e12 * e33 +
            e31 * e12 * e23 -
            e31 * e22 * e13;
}

inline Matrix3x3 Matrix3x3::Transpose(void)
{
    return Matrix3x3(e11, e21, e31, e12, e22, e32, e13, e23, e33);
}

inline Matrix3x3 Matrix3x3::Inverse(void)
{
    float d =   e11 * e22 * e33 -
                e11 * e32 * e23 +
                e21 * e32 * e13 -
                e21 * e12 * e33 +
                e31 * e12 * e23 -
                e31 * e22 * e13;

    if (d == 0)
        d = 1;

    return Matrix3x3(   (e22 * e33 - e23 * e32) / d,
                        -(e12 * e33 - e13 * e32) / d,
                        (e12 * e23 - e13 * e22) / d,
                        -(e21 * e33 - e23 * e31) / d,
                        (e11 * e33 - e13 * e31) / d,
                        -(e11 * e23 - e13 * e21) / d,
                        (e21 * e32 - e22 * e31) / d,
                        -(e11 * e32 - e12 * e31) / d,
                        (e11 * e22 - e12 * e21) / d );
}

inline Matrix3x3& Matrix3x3::operator+=(Matrix3x3 m)
{
    e11 += m.e11;
    e12 += m.e12;
    e13 += m.e13;
    e21 += m.e21;
    e22 += m.e22;
    e23 += m.e23;
    e31 += m.e31;
    e32 += m.e32;
    e33 += m.e33;
    return *this;
}

inline Matrix3x3& Matrix3x3::operator-=(Matrix3x3 m)
{
    e11 -= m.e11;
    e12 -= m.e12;
    e13 -= m.e13;
    e21 -= m.e21;
    e22 -= m.e22;
    e23 -= m.e23;
    e31 -= m.e31;
    e32 -= m.e32;
    e33 -= m.e33;
    return *this;
}

inline Matrix3x3& Matrix3x3::operator*=(float s)
{
    e11 *= s;
    e12 *= s;
    e13 *= s;
    e21 *= s;
    e22 *= s;
    e23 *= s;
    e31 *= s;
    e32 *= s;
    e33 *= s;
    return *this;
}

inline Matrix3x3& Matrix3x3::operator/=(float s)
{
    e11 /= s;
    e12 /= s;
    e13 /= s;
    e21 /= s;
    e22 /= s;
    e23 /= s;
    e31 /= s;
    e32 /= s;
    e33 /= s;
    return *this;
}

inline Matrix3x3 operator+(Matrix3x3 m1, Matrix3x3 m2)
{
    return Matrix3x3(   m1.e11 + m2.e11,
                        m1.e12 + m2.e12,
                        m1.e13 + m2.e13,
                        m1.e21 + m2.e21,
                        m1.e22 + m2.e22,
                        m1.e23 + m2.e23,
                        m1.e31 + m2.e31,
                        m1.e32 + m2.e32,
                        m1.e33 + m2.e33);
}

inline Matrix3x3 operator-(Matrix3x3 m1, Matrix3x3 m2)
{
    return Matrix3x3(   m1.e11 - m2.e11,
                        m1.e12 - m2.e12,
                        m1.e13 - m2.e13,
                        m1.e21 - m2.e21,
                        m1.e22 - m2.e22,
                        m1.e23 - m2.e23,
                        m1.e31 - m2.e31,
                        m1.e32 - m2.e32,
                        m1.e33 - m2.e33);
}

inline Matrix3x3 operator/(Matrix3x3 m, float s)
{
    return Matrix3x3(   m.e11 / s,
                        m.e12 / s,
                        m.e13 / s,
                        m.e21 / s,
                        m.e22 / s,
                        m.e23 / s,
                        m.e31 / s,
                        m.e32 / s,
                        m.e33 / s);
}

inline Matrix3x3 operator*(Matrix3x3 m1, Matrix3x3 m2)
{
    return Matrix3x3(   m1.e11 * m2.e11 + m1.e12 * m2.e21 + m1.e13 * m2.e31,
                        m1.e11 * m2.e12 + m1.e12 * m2.e22 + m1.e13 * m2.e32,
                        m1.e11 * m2.e13 + m1.e12 * m2.e23 + m1.e13 * m2.e33,
                        m1.e21 * m2.e11 + m1.e22 * m2.e21 + m1.e23 * m2.e31,
                        m1.e21 * m2.e12 + m1.e22 * m2.e22 + m1.e23 * m2.e32,
                        m1.e21 * m2.e13 + m1.e22 * m2.e23 + m1.e23 * m2.e33,
                        m1.e31 * m2.e11 + m1.e32 * m2.e21 + m1.e33 * m2.e31,
                        m1.e31 * m2.e12 + m1.e32 * m2.e22 + m1.e33 * m2.e32,
                        m1.e31 * m2.e13 + m1.e32 * m2.e23 + m1.e33 * m2.e33 );
}

inline Matrix3x3 operator*(Matrix3x3 m, float s)
{
    return Matrix3x3(   m.e11 * s,
                        m.e12 * s,
                        m.e13 * s,
                        m.e21 * s,
                        m.e22 * s,
                        m.e23 * s,
                        m.e31 * s,
                        m.e32 * s,
                        m.e33 * s);
}

inline Matrix3x3 operator*(float s, Matrix3x3 m)
{
    return Matrix3x3(   m.e11 * s,
                        m.e12 * s,
                        m.e13 * s,
                        m.e21 * s,
                        m.e22 * s,
                        m.e23 * s,
                        m.e31 * s,
                        m.e32 * s,
                        m.e33 * s);
}

inline Vector operator*(Matrix3x3 m, Vector u)
{
    return Vector(  m.e11 * u.x + m.e12 * u.y + m.e13 * u.z,
                    m.e21 * u.x + m.e22 * u.y + m.e23 * u.z,
                    m.e31 * u.x + m.e32 * u.y + m.e33 * u.z);
}

inline Vector operator*(Vector u, Matrix3x3 m)
{
    return Vector(  u.x * m.e11 + u.y * m.e21 + u.z * m.e31,
                    u.x * m.e12 + u.y * m.e22 + u.z * m.e32,
                    u.x * m.e13 + u.y * m.e23 + u.z * m.e33);
}

//------------------------------------------------------------------------//
// Quaternion Class and Quaternion functions
//------------------------------------------------------------------------//

class Quaternion
{
public:
    float n; // number (scalar) part
    Vector v; // vector part: v.x, v.y, v.z

    Quaternion(void);
    Quaternion(float e0, float e1, float e2, float e3);

    float Magnitude(void);
    Vector GetVector(void);
    float GetScalar(void);
    Quaternion operator+=(Quaternion q);
    Quaternion operator-=(Quaternion q);
    Quaternion operator*=(float s);
    Quaternion operator/=(float s);
    Quaternion operator~(void) const {
        return Quaternion(n, -v.x, -v.y, -v.z);
    }
};

inline Quaternion operator+(Quaternion q1, Quaternion q2);
inline Quaternion operator-(Quaternion q1, Quaternion q2);
inline Quaternion operator*(Quaternion q1, Quaternion q2);
inline Quaternion operator*(Quaternion q, float s);
inline Quaternion operator*(float s, Quaternion q);
inline Quaternion operator*(Quaternion q, Vector v);
inline Quaternion operator*(Vector v, Quaternion q);
inline Quaternion operator/(Quaternion q, float s);
inline float QGetAngle(Quaternion q);
inline Vector QGetAxis(Quaternion q);
inline Quaternion QRotate(Quaternion q1, Quaternion q2);
inline Vector QVRotate(Quaternion q, Vector v);
inline Quaternion MakeQFromEulerAngles(float x, float y, float z);
inline Vector MakeEulerAnglesFromQ(Quaternion q);


inline Quaternion::Quaternion(void)
{
    n = 0;
    v.x = 0;
    v.y =  0;
    v.z = 0;
}

inline Quaternion::Quaternion(float e0, float e1, float e2, float e3)
{
    n = e0;
    v.x = e1;
    v.y = e2;
    v.z = e3;
}

inline float Quaternion::Magnitude(void)
{
    return (float) sqrt(n * n + v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vector Quaternion::GetVector(void)
{
    return Vector(v.x, v.y, v.z);
}

inline float Quaternion::GetScalar(void)
{
    return n;
}

inline Quaternion Quaternion::operator+=(Quaternion q)
{
    n += q.n;
    v.x += q.v.x;
    v.y += q.v.y;
    v.z += q.v.z;
    return *this;
}

inline Quaternion Quaternion::operator-=(Quaternion q)
{
    n -= q.n;
    v.x -= q.v.x;
    v.y -= q.v.y;
    v.z -= q.v.z;
    return *this;
}

inline Quaternion Quaternion::operator*=(float s)
{
    n *= s;
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return *this;
}

inline Quaternion Quaternion::operator/=(float s)
{
    n /= s;
    v.x /= s;
    v.y /= s;
    v.z /= s;
    return *this;
}

/*inline Quaternion Quaternion::operator~()
{
return Quaternion(n, -v.x, -v.y, -v.z);
}*/

inline Quaternion operator+(Quaternion q1, Quaternion q2)
{
    return Quaternion(  q1.n + q2.n,
                        q1.v.x + q2.v.x,
                        q1.v.y + q2.v.y,
                        q1.v.z + q2.v.z);
}

inline Quaternion operator-(Quaternion q1, Quaternion q2)
{
    return Quaternion(  q1.n - q2.n,
                        q1.v.x - q2.v.x,
                        q1.v.y - q2.v.y,
                        q1.v.z - q2.v.z);
}

inline Quaternion operator*(Quaternion q1, Quaternion q2)
{
    return Quaternion(  q1.n * q2.n - q1.v.x * q2.v.x - q1.v.y * q2.v.y - q1.v.z * q2.v.z,
                        q1.n * q2.v.x + q1.v.x * q2.n + q1.v.y * q2.v.z - q1.v.z * q2.v.y,
                        q1.n * q2.v.y + q1.v.y * q2.n + q1.v.z * q2.v.x - q1.v.x * q2.v.z,
                        q1.n * q2.v.z + q1.v.z * q2.n + q1.v.x * q2.v.y - q1.v.y * q2.v.x);
}

inline Quaternion operator*(Quaternion q, float s)
{
    return Quaternion(q.n * s, q.v.x * s, q.v.y * s, q.v.z * s);
}

inline Quaternion operator*(float s, Quaternion q)
{
    return Quaternion(q.n * s, q.v.x * s, q.v.y * s, q.v.z * s);
}

inline Quaternion operator*(Quaternion q, Vector v)
{
    return Quaternion(  -(q.v.x * v.x + q.v.y * v.y + q.v.z * v.z),
                        q.n * v.x + q.v.y * v.z - q.v.z * v.y,
                        q.n * v.y + q.v.z * v.x - q.v.x * v.z,
                        q.n * v.z + q.v.x * v.y - q.v.y * v.x);
}

inline Quaternion operator*(Vector v, Quaternion q)
{
    return Quaternion(  -(q.v.x * v.x + q.v.y * v.y + q.v.z * v.z),
                        q.n * v.x + q.v.z * v.y - q.v.y * v.z,
                        q.n * v.y + q.v.x * v.z - q.v.z * v.x,
                        q.n * v.z + q.v.y * v.x - q.v.x * v.y);
}

inline Quaternion operator/(Quaternion q, float s)
{
    return Quaternion(q.n / s, q.v.x / s, q.v.y / s, q.v.z / s);
}

inline float QGetAngle(Quaternion q)
{
    return (float) (2 * acosf(q.n));
}

inline Vector QGetAxis(Quaternion q)
{
    Vector v;
    float m;

    v = q.GetVector();
    m = v.Magnitude();

    if (m <= tol)
        return Vector();
    else
        return v / m;
}

inline Quaternion QRotate(Quaternion q1, Quaternion q2)
{
    return q1 * q2 * (~q1);
}

inline Vector QVRotate(Quaternion q, Vector v)
{
    Quaternion t;


    t = q * v * (~q);

    return t.GetVector();
}

inline Quaternion MakeQFromEulerAngles(float x, float y, float z)
{
    Quaternion q;
    double roll = DegreesToRadians(x);
    double pitch = DegreesToRadians(y);
    double yaw = DegreesToRadians(z);

    double cyaw, cpitch, croll, syaw, spitch, sroll;
    double cyawcpitch, syawspitch, cyawspitch, syawcpitch;

    cyaw = cos(0.5f * yaw);
    cpitch = cos(0.5f * pitch);
    croll = cos(0.5f * roll);
    syaw = sin(0.5f * yaw);
    spitch = sin(0.5f * pitch);
    sroll = sin(0.5f * roll);

    cyawcpitch = cyaw * cpitch;
    syawspitch = syaw * spitch;
    cyawspitch = cyaw * spitch;
    syawcpitch = syaw * cpitch;

    q.n = (float) (cyawcpitch * croll + syawspitch * sroll);
    q.v.x = (float) (cyawcpitch * sroll - syawspitch * croll);
    q.v.y = (float) (cyawspitch * croll + syawcpitch * sroll);
    q.v.z = (float) (syawcpitch * croll - cyawspitch * sroll);

    return q;
}

inline Vector MakeEulerAnglesFromQ(Quaternion q)
{
    double r11, r21, r31, r32, r33;
    double q00, q11, q22, q33;
    double tmp;
    Vector u;

    q00 = q.n * q.n;
    q11 = q.v.x * q.v.x;
    q22 = q.v.y * q.v.y;
    q33 = q.v.z * q.v.z;

    r11 = q00 + q11 - q22 - q33;
    r21 = 2 * (q.v.x * q.v.y + q.n * q.v.z);
    r31 = 2 * (q.v.x * q.v.z - q.n * q.v.y);
    r32 = 2 * (q.v.y * q.v.z + q.n * q.v.x);
    r33 = q00 - q11 - q22 + q33;

    tmp = fabs(r31);
    if (tmp > 0.999999) {
        double r12 = 2 * (q.v.x * q.v.y - q.n * q.v.z);
        double r13 = 2 * (q.v.x * q.v.z + q.n * q.v.y);

        u.x = RadiansToDegrees(0.0f); //roll
        u.y = RadiansToDegrees((float) (-(pi / 2) * r31 / tmp)); // pitch
        u.z = RadiansToDegrees((float) atan2(-r12, -r31 * r13)); // yaw
        return u;
    }

    u.x = RadiansToDegrees((float) atan2(r32, r33)); // roll
    u.y = RadiansToDegrees((float) asinf(-r31));     // pitch
    u.z = RadiansToDegrees((float) atan2(r21, r11)); // yaw
    return u;


}





#endif
