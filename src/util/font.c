
#define M_FONT_DIV 16

void m_font_draw(struct m_image *dest, const struct m_image *src, const char *text, int max_len, int ox, int oy, float *color)
{
   int i;
   int len = strlen(text);
   int xs = src->width / M_FONT_DIV;
   int ys = src->height / M_FONT_DIV;
   int xd = xs;//7;
   int yd = ys+1;//10;
   int px = ox;
   int py = oy;

   if (max_len == 0 || len == 0)
      return;

   if (max_len > 0)
      len = M_MIN(max_len, len);

   for (i = 0; i < len; i++) {

      float *src_data, *src_pixel;
      float *dest_data, *dest_pixel;
      int ix, iy, x, y;
      unsigned char c = (unsigned char)text[i];

      if (c == '\n') {
         px = ox;
         py += yd;
         continue;
      }

      if (px + xs > dest->width) {
         px = ox;
         py += yd;
      }

      if (py + ys > dest->height)
         break;

      iy = c / M_FONT_DIV;
      ix = c - iy * M_FONT_DIV;
      ix *= xs;
      iy *= ys;

      dest_data = (float *)(dest->data) + (py * dest->width + px) * 3;
      src_data = (float *)(src->data) + (iy * src->width + ix) * 4;

      for (y = 0; y < ys; y++) {

         dest_pixel = dest_data + y * dest->width * 3;
         src_pixel = src_data + y * src->width * 4;

         for (x = 0; x < xs; x++) {

            if (src_pixel[3] > 0.5f) {
               dest_pixel[0] = color[0];
               dest_pixel[1] = color[1];
               dest_pixel[2] = color[2];
            }

            dest_pixel += 3;
            src_pixel += 4;
         }
      }

      px += xd;
   }
}
