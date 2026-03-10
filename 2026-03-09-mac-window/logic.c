#include "math.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct Point Point;
struct Point {
    int x;
    int y;
};

typedef struct Rgba Rgba;
struct Rgba {
    int r;
    int g;
    int b;
    int a;
};

void draw_pixel(unsigned char *data, int width, int height, Point p, Rgba color)
{
    int index = (p.x + (p.y * width)) * 4;
    data[index] = color.r;
    data[index+1] = color.g;
    data[index+2] = color.b;
    data[index+3] = color.a;
}

typedef struct Linterp Linterp;
struct Linterp
{
    int i;
    int opl;
    float d;
    float a;
};

Linterp linterp_init(int i0, int d0, int i1, int d1)
{
    Linterp result = {0};
    if (i0 == i1) {
        result.d = (float)d0;
        result.a = 0.0;
        result.opl = i0 + 1;
        result.i = i0;
    } else {
        result.i = i0;
        result.opl = i1+1;
        result.a = ((float)d1 - (float)d0)/((float)i1-(float)i0);
        result.d = (float)d0;
    }
    return result;
}

void linterp_next(Linterp *linterp)
{
    linterp->i++;
    linterp->d += linterp->a;
}


void draw_line(unsigned char *data, int width, int height, Point p0, Point p1)
{
    int dx = p1.x - p0.x;
    int dy = p1.y - p0.y;
    Rgba color_black = {0};

    // pick the axis with more values to iterate through
    // in generalized terms we distinguish between independen and dependent variable
    // and then interpolate the dependent based on the range of the independent
    if (abs(dx) > abs(dy)) {
        int x0 = p0.x;
        int x1 = p1.x;
        if (p1.x < p0.x) {
            x0 = p1.x;
            x1 = p0.x;
        }

        Linterp linterp = linterp_init(x0, p0.y, x1, p1.y);

        while (linterp.i != linterp.opl) {
            Point p = {.x=linterp.i, .y=(int)lround(linterp.d)};
            draw_pixel(data, width, height, p, color_black);
            linterp_next(&linterp);
        }

    } else {
        int y0 = p0.y;
        int y1 = p1.y;
        if (p1.y < p0.y) {
            y0 = p1.y;
            y1 = p0.y;
        }

        Linterp linterp = linterp_init(y0, p0.x, y1, p1.x);

        while (linterp.i != linterp.opl) {
            Point p = {.y=linterp.i, .x=(int)lround(linterp.d)};
            draw_pixel(data, width, height, p, color_black);
            linterp_next(&linterp);
        }
    }
}

// This is the function we will swap out
void update_buffer(unsigned char* data, int width, int height, int frame) {
    for (int i = 0; i < width * height * 4; i += 4) {
        data[i+0] = 150;                        // Red
        data[i+1] = 50;                         // Green
        data[i+2] = 200;                        // Blue
        data[i+3] = 255;                        // Alpha
    }

    Point p0 = {.x=602, .y=12};
    Point p1 = {.x=20, .y=200};
    draw_line(data, width, height, p0, p1);
    p0.x=44; p0.y=44;
    p1.x=60; p1.y=500;
    draw_line(data, width, height, p0, p1);
    p0.x=104; p0.y=44;
    p1.x=65; p1.y=60;
    draw_line(data, width, height, p0, p1);
    p0.x=104; p0.y=44;
    p1.x=104; p1.y=560;
    draw_line(data, width, height, p0, p1);
    p0.x=104; p0.y=44;
    p1.x=4; p1.y=44;
    draw_line(data, width, height, p0, p1);
}
