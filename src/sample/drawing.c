// EXAMPLE: https://github.com/anael-seghezzi/CToy/wiki/Displaying-and-drawing

#include <ctoy.h>
#include <m_raster.h> // use m_raster to draw primitives

struct m_image image = M_IMAGE_IDENTITY(); // initialize the struct (all set to zero in this case)

void ctoy_begin(void)
{
   int width = 256;
   int height = 256;
   int comp = 3; // RGB

   // create image (allocates memory)
   m_image_create(&image, M_FLOAT, width, height, comp);


   float *pixel = (float *)image.data; // we cast image data to float in this case
   int y, x;

   for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
         
         // write all pixels to green (we are in float, linear color space)
         pixel[0] = 1.0; // red
         pixel[1] = 1.0; // green
         pixel[2] = 1.0; // blue

         // go to next pixel
         pixel += comp;
      }
   }

   // display the image to the frame buffer
   // float linear is converted to ubyte sRGB on the fly
   ctoy_swap_buffer(&image);
}

void ctoy_end(void)
{
   // free image memory
   m_image_destroy(&image);
}

void ctoy_main_loop(void)
{
   // declare static variables
   static int prev_x;
   static int prev_y;

   // transform mouse coordinates into the image pixel coordinates
   int x = ctoy_mouse_x() * image.width;
   int y = ctoy_mouse_y() * image.height;

   // let's draw pixels when the mouse button is pressed
   if (ctoy_mouse_button_pressed(CTOY_MOUSE_BUTTON_LEFT)) {

      // be sure pixel coordinates are not out of range
      if (x >= 0 && x < image.width && y >= 0 && y < image.height) {

         // draw a single pixel if the mouse button was just pressed
         if (ctoy_mouse_button_press(CTOY_MOUSE_BUTTON_LEFT)) {

            float *pixel = (float *)image.data + (y * image.width + x) * image.comp;

            pixel[0] = 0.0;
            pixel[1] = 0.0;
            pixel[2] = 0.0;
         }
         // otherwise draw a line between previous and current pixels
         else {

            float p0[2] = {prev_x, prev_y};
            float p1[2] = {x, y};
            float color[3] = {0.0, 0.0, 0.0};

            m_raster_line(
               (float *)image.data, image.width, image.height, image.comp,
               p0, p1,
               color);
         }

         // remember previous pixel coordinates
         prev_x = x;
         prev_y = y;

         ctoy_swap_buffer(&image);
      }
   }
}