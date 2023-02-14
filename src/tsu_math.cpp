#include "tsu_math.h"


Vec2 operator+(Vec2 a, Vec2 b){
    return(Vec2{a.x + b.x, a.y + b.y});
}
Vec2 operator-(Vec2 a, Vec2 b){
    return(Vec2{a.x - b.x, a.y - b.y});
}
Vec2 operator*(Vec2 a, float b){
    return(Vec2{a.x * b, a.y *b});
}
Vec2 operator*(float b, Vec2 a){
    return(Vec2{a.x * b, a.y *b});
}
Vec2 operator/(Vec2 a, float b){
    return(Vec2{a.x/b, a.y/b});
}
Vec2 operator-(Vec2 a){
    return(Vec2{-a.x, -a.y});
}
void operator+=(Vec2& a, Vec2 b){
    a = a+b;
}
void operator-=(Vec2& a, Vec2 b){
    a = a-b;
}
void operator*=(Vec2& a, float b){
    a = a*b;
}
void operator/=(Vec2& a, float b){
    a = a/b;
}

float lenSquared(Vec2 a){
    return(a.x * a.x + a.y * a.y);
}
float len(Vec2 a){
    return(sqRoot(lenSquared(a)));
}

float dot(Vec2 a, Vec2 b){
    return(a.x * b.x + a.y * b.y);
}

// clockwise = +ve; and anticlockwise = -ve
float crossMag(Vec2 a, Vec2 b){
    return(a.x * b.y - b.x * a.y);
}

Vec2 normalize(Vec2 a){
    return(a/len(a));
}
float projection(Vec2 of, Vec2 on){
    return(dot(of, normalize(on)));
}

// quick reverse square root from quake III 
float Q_rsqrt(float number)              
{
    int i;
    float x2, y;
    const float threehalfs = 1.5f;
    x2 = number * 0.5f;
    y  = number;
    i  = *(int*) &y;
    i  = 0x5f3759df - ( i >> 1 );
    y  = *(float*)&i;
    y  = y * (threehalfs - (x2 * y * y));   // 1st iteration
	//  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed
    return y;
}

float fastSqRoot(float number){
    return(Q_rsqrt(number) * number);
}