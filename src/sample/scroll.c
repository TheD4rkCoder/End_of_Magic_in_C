#include <ctoy.h>
#include "../util/font.c"
#include "../util/img_utils.c"

#define BUF_WIDTH 256
#define BUF_HEIGHT 256

struct m_image buffer = M_IMAGE_IDENTITY();
struct m_image image = M_IMAGE_IDENTITY();
struct m_image cutout = M_IMAGE_IDENTITY();

// cutout positions of picture
int x = 0;
int y = 0;
int w = 32; // width
int h = 32; // height

void draw(struct m_image *dest)
{  
   // load cutout of picture
   m_image_copy_sub_image(&cutout, &image, x, y, w, h);

   // resize image (x1.25)
   m_image_resize(&cutout, &cutout, w*1.25, h*1.25);

   // draw position of selected sub-part (cutout.width/cutout.height after resize, also w/h before)
   int dx = (BUF_WIDTH - cutout.width) / 2;
   int dy = (BUF_HEIGHT - cutout.height) / 2;

   // draw sup-part of picture (to buffer) on position
   m_image_draw(dest, &cutout, dx, dy);

   y += 1;

   if(y > image.height - h) // jumping-change ...
   {
      y = 0;
   }
}

void ctoy_begin(void)
{
   ctoy_window_size(512, 512);

   m_image_load_float(&image, "data/numbers.png");
   
   m_image_create(&buffer, M_FLOAT, BUF_WIDTH, BUF_HEIGHT,  3);
}

void ctoy_end(void)
{
   m_image_destroy(&buffer);
}

void ctoy_main_loop(void)
{  
   draw(&buffer);

   ctoy_swap_buffer(&buffer);
}
