#include "tsu_math.h"
#include "SDL.h"

#define Renderer SDL_Renderer

int getRandom() {
    int a[] = {
        787,122,853,482,42,567,162,404,722,427,
2,840,678,305,375,307,866,639,828,530,337,
3,439,759,311,427,322,21,131,235,239,484,
4,994,529,332,364,508,579,91,3,950,952,
5,127,598,123,898,139,932,466,611,917,956,
6,195,269,204,971,519,231,268,895,83,787,
7,657,786,525,424,580,530,867,931,75,383,
8,913,100,914,29,952,861,359,178,454,668,
9,633,937,919,824,346,231,18,44,843,21,
10,319,499,272,147,415,811,325,966,658,591,
11,307,926,857,231,974,727,131,933,892,38,
12,58,635,722,528,383,693,907,242,924,383,
13,784,498,281,389,288,677,691,333,800,858,
14,670,278,58,249,94,160,891,659,731,595,
15,612,43,147,675,520,11,877,893,991,775,
16,204,366,511,614,8,132,847,356,451,96,
17,964,471,551,384,765,356,113,535,607,849,
18,999,86,804,359,165,649,762,386,671,136,
19,672,644,750,451,273,860,121,634,378,359,
20,103,950,612,667,72,570,350,794,884,277,
21,18,693,890,243,883,804,717,628,233,456,
22,897,635,287,953,598,162,99,82,26,282,
23,192,441,50,710,1,946,664,367,187,696,
24,392,344,695,245,770,754,507,912,556,25,
25,270,798,216,673,183,36,956,325,509,876,
26,199,262,702,903,388,867,511,626,16,9,
27,646,864,203,407,974,125,463,175,536,521,
28,928,468,593,485,519,197,405,844,377,334,
29,100,521,324,921,431,12,635,884,301,635,
30,693,79,693,679,349,765,627,515,590,859,
31,937,784,932,272,298,986,485,577,730,306,
32,593,588,639,119,593,154,915,997,310,973,
33,462,404,710,126,89,899,78,78,948,805,
34,380,490,280,484,773,887,478,883,394,290,
35,662,197,329,219,327,142,920,15,882,150,
36,845,630,548,960,867,125,758,0,628,872,
37,727,777,230,871,581,312,523,276,852,359,
38,464,864,90,488,162,755,154,879,993,317,
39,369,775,883,710,9,703,54,854,438,702,
40,45,902,898,733,806,754,38,436,275,322,
41,54,916,42,52,290,672,506,560,135,228,
42,134,834,997,309,695,914,510,996,735,716,
43,770,136,592,102,708,348,772,64,800,288,
44,954,50,863,267,665,813,600,305,48,746,
45,105,421,393,468,973,119,225,251,914,18,
46,162,870,995,353,595,176,989,973,732,512,
47,746,500,917,296,923,320,588,120,220,294,
48,534,415,986,173,298,933,211,909,296,258,
49,272,469,746,18,609,49,820,710,297,950,
50,304,571,919,665,140,477,915,484,496,280,

    };
    static int i = 0;
    if (i >= 549)
        i = 0;
    return(a[i++]);
}


typedef Vector3<uint8_t> Color;

int LCGrandom(int a, int b, int m, int seed){
    return((a * seed + b)%m);
}

int drawLine(Renderer* ren, Vec2 start, Vec2 end, Color c = { 0,0,0 }) {
    SDL_SetRenderDrawColor(ren, c.x, c.y, c.z, 255);
    SDL_RenderDrawLineF(ren, start.x, start.y, end.x, end.y);
    return(0);
}

void quadBezierCurve(Renderer * ren, Vec2 p0, Vec2 p1, Vec2 p2){
    Vec2 current = p0;
    float t = 0;
    while (t != 1){
        t = Clamp(0, t + 0.05, 1);
        Vec2 a = lerp<Vec2>(p0, p1, t);
        Vec2 b = lerp<Vec2>(p1, p2, t);
        Vec2 p = lerp<Vec2>(a, b, t);
        drawLine(ren, current, p);
        current = p;
    }
}

void cubicBezierCurve(Renderer * ren, Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3){
    Vec2 current = p0;
    float t = 0;
    while (t != 1){
        t = Clamp(0, t + 0.05, 1);
        Vec2 a = lerp<Vec2>(p0, p1, t);
        Vec2 b = lerp<Vec2>(p1, p2, t);
        Vec2 c = lerp<Vec2>(p2, p3, t);

        Vec2 d = lerp<Vec2>(a, b, t);
        Vec2 e = lerp<Vec2>(b, c, t);

        Vec2 p = lerp<Vec2>(d, e, t);
        drawLine(ren, current, p);
        current = p;
    }
}


int drawTriangle(Renderer * ren, Triangle * t, uint8_t r, uint8_t g, uint8_t b){
    SDL_SetRenderDrawColor(ren, r, g, b, 255);
    drawLine(ren, t->a, t->b);
    drawLine(ren, t->b, t->c);
    drawLine(ren, t->c, t->a);
    return(0);
}

int drawCircle(Renderer* ren, Circle* c, Color color = {0,0,0}) {
    // 1.3375 factor used by manually making circle using cubic bezier curve and checking in krita
    Rect controlPoints = {c->center.x - c->r, c->center.y - c->r * 1.3375f, 2.0f*c->r, 2.0f*1.3375f*c->r};
    SDL_SetRenderDrawColor(ren, color.x, color.y, color.z, 255);
    /*quadBezierCurve(ren, {controlPoints.x, controlPoints.y + 0.5f*controlPoints.h}, {controlPoints.x, controlPoints.y}, {controlPoints.x + 0.5f*controlPoints.w, controlPoints.y});
    quadBezierCurve(ren, {controlPoints.x + 0.5f*controlPoints.w, controlPoints.y}, {controlPoints.x + controlPoints.w, controlPoints.y}, {controlPoints.x + controlPoints.w, controlPoints.y + 0.5f*controlPoints.h});
    quadBezierCurve(ren, {controlPoints.x + controlPoints.w, controlPoints.y + 0.5f*controlPoints.h}, {controlPoints.x + controlPoints.w, controlPoints.y + controlPoints.h}, {controlPoints.x + 0.5f*controlPoints.w, controlPoints.y + controlPoints.h});
    quadBezierCurve(ren, {controlPoints.x + 0.5f*controlPoints.w, controlPoints.y + controlPoints.h}, {controlPoints.x, controlPoints.y + controlPoints.h}, {controlPoints.x, controlPoints.y + 0.5f*controlPoints.h});*/
    cubicBezierCurve(ren, { controlPoints.x, controlPoints.y + 0.5f * controlPoints.h }, { controlPoints.x, controlPoints.y }, { controlPoints.x + controlPoints.w, controlPoints.y }, { controlPoints.x + controlPoints.w, controlPoints.y + 0.5f * controlPoints.h });
    cubicBezierCurve(ren, {controlPoints.x + controlPoints.w, controlPoints.y + 0.5f*controlPoints.h}, {controlPoints.x + controlPoints.w, controlPoints.y + controlPoints.h}, {controlPoints.x, controlPoints.y + controlPoints.h}, {controlPoints.x, controlPoints.y + 0.5f*controlPoints.h});
    return(0);
}

int drawShape(Renderer* ren, Shape* s, Color color) {
    SDL_SetRenderDrawColor(ren, color.x, color.y, color.z, 255);
    for (int i=0; i<s->n -1; i++){
        drawLine(ren, s->points[i], s->points[i+1]);
    }
    drawLine(ren, s->points[s->n - 1], s->points[0]);
    return(0);
}

void renderHighlightPoint(Renderer* ren, Vec2* point, Color color = {0,0,0}) {
    Circle c = {*point, 5};
    SDL_SetRenderDrawColor(ren, 0,0,0,255);
    SDL_RenderDrawPointF(ren, point->x, point->y);
    drawCircle(ren, &c, color);

}

