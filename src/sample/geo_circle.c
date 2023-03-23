#include <ctoy.h>
#include <m_math.h>
#include <m_image.h>
#include <m_raster.h>
#include "../util/img_util.c"

#define WINDOW_SIZE 128
#define LOOP 10

struct m_image buffer = M_IMAGE_IDENTITY();

void image_draw(void)
{
    for(int i = 0; i <= LOOP; i++){
        
        float color[3];

        for(int c = 0; c < 3; c++){
            color[c] = m_randf();
        }

        // point center
        float2 p[2];
        p[0].x = buffer.width/2;
        p[0].y = buffer.height/2;

        float radius = (i * (buffer.height/2/LOOP));

        // draw circle on buffer
        m_raster_circle(buffer.data, buffer.width, buffer.height, buffer.comp, &p[0].x, radius , color);
    }
}

void ctoy_begin(void)
{
    ctoy_window_size(512, 512);

    m_image_create(&buffer, M_FLOAT, WINDOW_SIZE, WINDOW_SIZE, 3);
}

void ctoy_end(void)
{
   m_image_destroy(&buffer);
}

void ctoy_main_loop(void)
{
    image_draw();

    ctoy_swap_buffer(&buffer);
}
