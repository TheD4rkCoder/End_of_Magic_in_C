#include <ctoy.h>
#include "../util/font.c"
#include "../util/img_utils.c"

#define BUF_WIDTH 128
#define BUF_HEIGHT 128

struct m_image buffer = M_IMAGE_IDENTITY();

struct m_image img_planet = M_IMAGE_IDENTITY();
struct m_image planet = M_IMAGE_IDENTITY();

struct m_image img_idle = M_IMAGE_IDENTITY();
struct m_image idle[2] = M_IMAGE_IDENTITY();

struct m_image img_run = M_IMAGE_IDENTITY();
struct m_image run[4] = M_IMAGE_IDENTITY();

int changePicture = 0;

void draw(struct m_image *dest)
{
    // clear buffer before redraw
    memset(buffer.data, 0, buffer.size * sizeof(float));

    float2 p;
    p.x = (buffer.width - 64 ) / 2; 
    p.y = (buffer.height / 2 );

    // rotate
    //m_image_rotate_left(&planet, &planet);

    m_image_draw(dest, &planet, p.x, p.y);

    int x = (buffer.width - 32) / 2;
    int y = (buffer.height - 32) / 3;

    // timing ?
    int t = ctoy_get_time() * 5;
    if(t % 2) {
        changePicture++;
    }

    // switch between two animation steps
    if(changePicture % 2) {
        m_image_draw(dest, &idle[0], x, y);
    } else {
        m_image_draw(dest, &idle[1], x, y);
    }
}

void ctoy_begin(void)
{
    ctoy_window_size(512, 512);

    // planet
    m_image_load_float(&img_planet, "data/celestial_objects.png");
    // load parts
    m_image_copy_sub_image(&planet, &img_planet, 64, 0, 64, 64);

    // character idle
    m_image_load_float(&img_idle, "data/mr_mochi_idle_32x32.png");
    // load parts
    m_image_copy_sub_image(&idle[0], &img_idle, 0, 0, 32, 32);
    m_image_copy_sub_image(&idle[1], &img_idle, 32, 0, 32, 32);

    // character run
    m_image_load_float(&img_run, "data/mr_mochi_running_32x32.png");
    // load parts
    int pos_x = 0;
    for(int i = 0; i < 4; i++){
        m_image_copy_sub_image(&run[0], &img_run, pos_x,  0, 32, 32);
        pos_x+=32;
    }

    m_image_create(&buffer, M_FLOAT, BUF_WIDTH, BUF_HEIGHT, 3);
}

void ctoy_end(void)
{
    m_image_destroy(&buffer);
    m_image_destroy(&img_idle);
    m_image_destroy(&idle[0]);
    m_image_destroy(&idle[1]);
}

void ctoy_main_loop(void)
{
    draw(&buffer);

    ctoy_swap_buffer(&buffer);
}
