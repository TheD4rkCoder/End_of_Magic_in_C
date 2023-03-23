#include <ctoy.h>
#include "../util/img_util.c"

#define PIXEL 16

struct m_image buffer = M_IMAGE_IDENTITY();

void pixel_init(void)
{
   float *pixel = (float *)buffer.data;
   for(int i = 0; i < (buffer.width * buffer.height * 3); i++){
      pixel[i] = 0.0;
   }
}


void pixel_draw(void)
{
   float *pixel = (float *)buffer.data;
   int y, x;

   for (y = 0; y < buffer.height; y++) {
      for (x = 0; x < buffer.width; x++) {
         if(x > y){
            pixel[0] = 1.0; // R
            pixel[1] = 1.0; // G
            pixel[2] = 1.0; // B
         } 
         pixel += 3;
      }
   }   
}

void ctoy_begin(void)
{
   ctoy_window_size(256, 256);

   m_image_create(&buffer, M_FLOAT, PIXEL, PIXEL, 3);

   pixel_init();
}

void ctoy_end(void)
{
   m_image_destroy(&buffer);
}

void ctoy_main_loop(void)
{
   pixel_draw();

   ctoy_swap_buffer(&buffer);
}
