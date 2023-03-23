#include <ctoy.h>
#include <string.h>
#include <m_math.h>
#include <m_color.h>
#include <m_image.h>
#include <m_raster.h>
#include "../util/font.c"
#include "../util/img_utils.c" // s

#define SIZE_WIDTH 256
#define SIZE_HEIGHT 256

struct m_image buffer = M_IMAGE_IDENTITY();
struct m_image animate_step = M_IMAGE_IDENTITY();
struct m_image animate_original = M_IMAGE_IDENTITY();
struct m_image font_default = M_IMAGE_IDENTITY();
struct m_image font_small = M_IMAGE_IDENTITY();

float color[3] = {1, 1, 1};

double ctime;
double _t;

// from: BackOnEarth
double get_elapsed_t()
{
	double t = ctoy_get_time();
	double elapst = t - ctime;
	ctime = t;
	return elapst;
}

void draw(double ft)
{ 
   // show how many chars?
   int maxString = 128;
   int length = (ft * 10 < maxString) ? (ft * 10) : maxString;
   // draw specified length of string
   m_font_draw(&buffer, &font_default, "abcdefghijklmnopqrstuvwxyzy0123456789abcdefghijklmnopqrstuvwxyzy.", length, 0, 0, color); 

   // magic width calculation
   int t = ctoy_t();
   float sint = sin(t * 0.01);
   int animate_width = M_ABS( sint * animate_original.width * 2) + 1;

   // clear animation step
   memset(animate_step.data, 0, animate_step.size * sizeof(float));
   // resize original by calculated width
   m_image_resize(&animate_step, &animate_original, animate_width, animate_step.height);
   // draw new animation step
   m_image_draw(&buffer, &animate_step, 0, SIZE_HEIGHT/2);
}

void ctoy_begin(void)
{
   ctoy_window_size(512, 512);

   int w = 256;

	// font
	m_image_load_float(&font_default, "data/font/default.tga");
	m_image_load_float(&font_small, "data/font/small.tga");

   // font animation
   m_image_create(&animate_original, M_FLOAT, SIZE_WIDTH/2, SIZE_HEIGHT/2, 3);
   // draw text on ...
   m_font_draw(&animate_original, &font_small, "ABCDEFGHIJKLMNOPQRSTUVWXYZ.", strlen("abcdefghijklmnopqrstuvwxyzy."), 0, 0, color);
   // font animation step
   m_image_create(&animate_step, M_FLOAT, SIZE_WIDTH/2, SIZE_HEIGHT/2, 3);

   // buffer
   m_image_create(&buffer, M_FLOAT, SIZE_WIDTH, SIZE_WIDTH, 3);
   
   printf("ctoy_frame_buffer_width : %d\n", ctoy_frame_buffer_width());
   printf("ctoy_frame_buffer_height: %d\n", ctoy_frame_buffer_height());
}

void ctoy_end(void)
{
   m_image_destroy(&buffer);
   m_image_destroy(&font_default);
   m_image_destroy(&font_small);
}

void ctoy_main_loop(void)
{
   double elapst = get_elapsed_t();
	_t += elapst;

   // clear buffer
   memset(buffer.data, 0, buffer.size * sizeof(float));
   // redraw
   draw(_t);
   // show
   ctoy_swap_buffer(&buffer);
}
