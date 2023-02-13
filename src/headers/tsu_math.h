#pragma once

#include <stdint.h>
// #include "define.h"

#define Min(a, b)           ((a<b)?a:b)
#define Max(a, b)           ((a>b)?a:b)
#define Clamp(a, val, b)    (Max(a, Min(val, b)))


#define sqRoot(X) fastSqRoot(X)

float Q_rsqrt(float number);
float fastSqRoot(float number);



template <class T>
struct Vector2{
	T x, y;
};
typedef Vector2<float> Vec2;

Vec2 operator+(Vec2, Vec2);
Vec2 operator-(Vec2, Vec2);
Vec2 operator*(Vec2, float);
Vec2 operator*(float, Vec2);
Vec2 operator/(Vec2, float);
Vec2 operator-(Vec2);
void operator+=(Vec2&, Vec2);
void operator-=(Vec2&, Vec2);
void operator*=(Vec2&, float);
void operator/=(Vec2&, float);

float lenSquared(Vec2);
float len(Vec2);

float dot(Vec2, Vec2);
float crossMag(Vec2, Vec2);

Vec2 normalize(Vec2);
float projection(Vec2 of, Vec2 on);

template <class T>
struct Vector3{
	T x, y, z;
};
typedef Vector3<float> Vec3;



struct Line{
	Vec2 start, dir;
	float len;
};

struct Rect{
	float x,y,w,h;
};

struct Circle{
	Vec2 center;
	float r;
};

struct Triangle{
	Vec2 a, b, c;
};

struct Shape{
	Vec2 *points;
	int n;
};

//int drawTriangle(Renderer * ren, Triangle * t, uint8_t r, uint8_t g, uint8_t b);

template <class T>
T lerp(T a, T b, float t){
	return(a+(b-a)*t);
}




