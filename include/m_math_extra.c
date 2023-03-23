
#define M_PI2 (M_PI * 2.0)
#define M_SQLENGHT2(v) ((v).x * (v).x + (v).y * (v).y)

#define M_DIV2_1(dest, A, B){ float l = B;\
   if (l > 0) { l = 1.0f / l; (dest).x = (A).x * l; (dest).y = (A).y * l; }\
   else { (dest).x = 0.0f; (dest).y = 0.0f; }}


void m_normalize_max(float *dest, const float *src, int size)
{
	int i; float m, max = src[0];
	for (i = 1; i < size; i++)
		max = M_MAX(max, src[i]);
	m = 1.0f / max;
	for (i = 0; i < size; i++)
		dest[i] = src[i] * m;
}

/* angle that a needs to rotate to meet b */
float m_2d_angle(float2 *a, float2 *b)
{
	float angle = atan2(b->y, b->x) - atan2(a->y, a->x);
	
	if(angle > M_PI)
		angle -= M_PI * 2;
	if(angle < -M_PI)
		angle += M_PI * 2;
	
	return angle;
}

void m_2d_rotate(float2 *dest, float2 *src, float angle)
{
	float x = src->x;
	float y = src->y;
	float sina = sinf(angle);
	float cosa = cosf(angle);
	dest->x = x * cosa - y * sina;
	dest->y = x * sina + y * cosa;
}

/* looped offset */
void m_array_offset(float *dest, const float *src, int size, int offset)
{
	int i;
	for (i = 0; i < size; i++) {
		int i2 = (i + offset) % size;
		dest[i] = src[i2];
	}
}

float m_max(const float *src, int count)
{
   float ma; int i;
   ma = src[0];
   for (i = 1; i < count; i++) {
      float v = src[i];
      ma = M_MAX(ma, v);
   }
   return ma;
}

void m_copy(float *dest, const float *src, int size)
{
	memcpy(dest, src, sizeof(float) * size);
}

void m_shuffle(int *dest, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		int id = m_rand() % size;
		int tmp = dest[i];
		dest[i] = dest[id];
		dest[id] = tmp;
	}
}

void m_min_max(float *min, float *max, const float *src, int count)
{
   float mi, ma; int i;
   
   mi = ma = src[0];
   for (i = 1; i < count; i++) {
      float v = src[i];
      mi = M_MIN(mi, v);
      ma = M_MAX(ma, v);
   }
   
   *min = mi;
   *max = ma;
}

void m_normalize_min_max(float *dest, const float *src, int count, float min, float max)
{
   float range, ir; int i;
   range = max - min;
   
   if (range < FLT_EPSILON) {
      memset(dest, 0, sizeof(float) * count);
      return;
   }
   
   ir = 1.0f / range;
   for (i = 0; i < count; i++)
      dest[i] = (src[i] - min) * ir;
}
