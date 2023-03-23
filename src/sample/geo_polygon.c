#include <ctoy.h>
#include <m_math.h>
#include <m_image.h>
#include <m_raster.h>
#include "../util/img_util.c"

#define WINDOW_SIZE 128

struct m_image buffer = M_IMAGE_IDENTITY();

float color[3] = { 1.0, 1.0, 1.0 };

void draw(void)
{ 
    // length/count of polygons
    int count = 4;

    // polygons
    float2 p[4];
    p[0].x = 0;
    p[0].y = buffer.height / 2;

    p[1].x = buffer.width / 2;
    p[1].y = buffer.height / 2;

    p[2].x = buffer.width;
    p[2].y = 0;

    p[3].x = buffer.width / 2;
    p[3].y = buffer.height;

    // draw polygon on buffer
    m_raster_polygon(buffer.data, buffer.width, buffer.height, 3, &p[0].x, count, color); 
}

void ctoy_begin(void)
{
    ctoy_window_title("Tunnel!");
    ctoy_window_size(512, 512);

    m_image_create(&buffer, M_FLOAT, WINDOW_SIZE, WINDOW_SIZE, 3);
}

void ctoy_end(void)
{
   m_image_destroy(&buffer);
}

void ctoy_main_loop(void)
{
    draw();

    ctoy_swap_buffer(&buffer);
}
