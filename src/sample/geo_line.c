#include <ctoy.h>
#include <m_math.h>
#include <m_image.h>
#include <m_raster.h>
#include "../util/img_util.c"

#define WINDOW_SIZE 128
#define LOOP 10

struct m_image buffer = M_IMAGE_IDENTITY();

void draw(void)
{
    for(int i = 0; i <= LOOP; i++){
        
        float color[3];
        
        for(int c = 0; c < 3; c++){
            color[c] = m_randf();
        }

        // point start
        float2 ps[2]; 
        ps[0].x = 0;
        ps[0].y = buffer.height/LOOP * (i + 1);

        // point end
        float2 pe[2];
        pe[0].x = buffer.width - 1;
        pe[0].y = buffer.height/LOOP * (i + 1);

        // draw line on buffer
        m_raster_line(buffer.data, buffer.width, buffer.height, 3, &ps[0].x, &pe[0].x, color); 
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
    draw();

    ctoy_swap_buffer(&buffer);
}
