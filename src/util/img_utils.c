#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#include "stb_image.h"

#include <assert.h>
#include <m_color.h>

int m_image_load(struct m_image *dest, const char *filename)
{
   int w, h, n;
   char *data = stbi_load(filename, &w, &h, &n, 0);

   if (data == NULL) {
      printf("ERROR CTOY: unable to read image %s\n", filename);
      return 0;
   }

   m_image_create(dest, M_UBYTE, w, h, n);
   memcpy(dest->data, data, dest->size * sizeof(char));
   stbi_image_free(data);

   return 1;
}

int m_image_load_float(struct m_image *dest, const char *filename)
{
   struct m_image ubi = M_IMAGE_IDENTITY();

   if (! m_image_load(&ubi, filename))
      return 0;

   m_image_ubyte_to_float(dest, &ubi);
   m_sRGB_to_linear((float*)dest->data, (float*)dest->data, dest->size);
   m_image_destroy(&ubi);
   return 1;
}

void m_image_grey_to_rgb(struct m_image *dest, const struct m_image *src)
{
	float *dest_p, *src_p;
	int i;
	
	m_image_create(dest, M_FLOAT, src->width, src->height, 3);
	dest_p = (float *)dest->data;
	src_p = (float *)src->data;

	for (i = 0; i < src->size; i++) {
		float v = *src_p;
		dest_p[0] = v;
		dest_p[1] = v;
		dest_p[2] = v;
		dest_p += 3;
		src_p++;
	}
}

void m_image_add(struct m_image *dest, const struct m_image *a, const struct m_image *b)
{
	int i;
	float *dest_p = (float *)dest->data;
	float *a_p = (float *)a->data;
	float *b_p = (float *)b->data;
	
	for (i = 0; i < a->size; i++)
		dest_p[i] = a_p[i] + b_p[i];
}

void m_image_sub(struct m_image *dest, const struct m_image *a, const struct m_image *b)
{
	int i;
	float *dest_p = (float *)dest->data;
	float *a_p = (float *)a->data;
	float *b_p = (float *)b->data;
	
	for (i = 0; i < a->size; i++)
		dest_p[i] = a_p[i] - b_p[i];
}

void m_image_invert(struct m_image *dest, const struct m_image *src)
{
	int i;
	float *dest_p = (float *)dest->data;
	float *src_p = (float *)src->data;
	
	for (i = 0; i < src->size; i++) {
		float v = src_p[i];
		dest_p[i] = 1.0f - M_CLAMP(v, 0.0f, 1.0f);
	}
}

void m_image_mul1(struct m_image *dest, const struct m_image *src, float mu)
{
	int i;
	float *dest_p = (float *)dest->data;
	float *src_p = (float *)src->data;
	
	for (i = 0; i < src->size; i++)
		dest_p[i] = src_p[i] * mu;
}

void m_image_add1(struct m_image *dest, const struct m_image *src, float a)
{
	int i;
	float *dest_p = (float *)dest->data;
	float *src_p = (float *)src->data;
	
	for (i = 0; i < src->size; i++)
		dest_p[i] = src_p[i] + a;
}

void m_raster_line_add(struct m_image *dest, float2 *p0, float2 *p1, float *color)
{
   float *data = (float *)dest->data;
   int x0 = (int)p0->x;
   int y0 = (int)p0->y;
   int x1 = (int)p1->x;
   int y1 = (int)p1->y;
   int w = dest->width;
   int h = dest->height;
   int comp = dest->comp;
   int dx =  M_ABS(x1 - x0), sx = x0 < x1 ? 1 : -1;
   int dy = -M_ABS(y1 - y0), sy = y0 < y1 ? 1 : -1;
   int err = dx + dy, e2;

   while (1) {

	 float *pixel = data + (y0 * w + x0) * comp; int c;
	 for (c = 0; c < comp; c++)
		pixel[c] += color[c];
      
      if (x0 == x1 && y0 == y1)
         break;
      
      e2 = 2 * err;
      if (e2 >= dy) { err += dy; x0 += sx; }
      if (e2 <= dx) { err += dx; y0 += sy; }
   }
}

void image_effect_rand(struct m_image *dest, int count)
{
	float *data = (float *)dest->data;
	int psize = dest->width * dest->height;
	int i;

	for (i = 0; i < count; i++) {

		float *p1, *p2;
		int r1 = m_rand() % psize;
		int r2 = m_rand() % psize;
		int c;

		p1 = data + r1 * dest->comp;
		p2 = data + r2 * dest->comp;

		for (c = 0; c < dest->comp; c++) {
			float tmp = p1[c];
			p1[c] = p2[c];
			p2[c] = tmp;
		}
	}
}

void m_image_draw(struct m_image *dest, const struct m_image *src, int px, int py)
{
	float *dest_p = (float *)dest->data + (py * dest->width + px) * dest->comp;
	float *src_p = (float *)src->data;
	int x, y;

	assert(dest->size > 0 && src->size > 0 && dest->comp >= 3 && src->comp >= 3);

	for (y = 0; y < src->height; y++) {
		
		float *pixel = dest_p;
		for (x = 0; x < src->width; x++) {

			if (src_p[3] > 0.5f) {
				pixel[0] = src_p[0];
				pixel[1] = src_p[1];
				pixel[2] = src_p[2];
			}

			src_p += src->comp;
			pixel += dest->comp;
		}

		dest_p += dest->width * dest->comp;
	}
}
