#pragma once

#include <gtc/matrix_transform.hpp>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Pos = glm::vec<2, float>;
using Mat = glm::mat<3, 3, float>;


Pos operator %(Pos base, Pos divisor) {
    Pos res;
    res.x = (((int)base.x % (int)divisor.x) + (int)divisor.x) % (int)divisor.x;
    res.y = (((int)base.y % (int)divisor.y) + (int)divisor.y) % (int)divisor.y;
    res.x += base.x - floor(base.x);
    res.y += base.y - floor(base.y);
    return res;
}

Pos operator %(Pos base, double divisor) {
    return base % Pos{ divisor,divisor };
}

Mat get_rot_mat(double angle) {
    return Mat{
        {cos(angle), sin(angle), 0.0},
        { -sin(angle),cos(angle),0.0 },
        { 0.0,0.0,1.0 }
    };
}
Mat get_delta_mat(Pos delta) {
    return Mat{
        {1.0, 0.0, 0.0},
        { 0.0,1.0,0.0 },
        { delta.x,delta.y,1.0 }
    };
}
Mat get_scale_mat(Pos scale) {
    return Mat{
        {scale.x, 0.0, 0.0},
        { 0.0,scale.y,0.0 },
        { 0.0,0.0,1.0 }
    };
}
Mat get_scale_mat(double scale) {
    return get_scale_mat({ scale,scale });
}

Pos transform(Mat mat, Pos coor) {
    Pos res = mat * Vec3{coor.x, coor.y, 1.0};
    return Pos{res.x, res.y};
}

Vec2 transform_vec(Mat mat, Vec2 coor) {
    Vec3 res = mat * Vec3{coor.x, coor.y, 0.0};
    return Vec2{res.x, res.y};
}