#include <ctoy.h>
#include "../util/img_util.c"
#include <m_image.h>
#include <m_raster.h>
#include "../util/font.c"
#include <stdbool.h>
//#include <C:/MinGW/include/sys/types.h>
//#include <C:/MinGW/include/time.h>

#define PIXEL 17

struct m_image buffer = M_IMAGE_IDENTITY();
struct m_image font_default = M_IMAGE_IDENTITY();
struct m_image font_small = M_IMAGE_IDENTITY();
bool withbot = 0;

int wincheck(char field[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        if ((field[i][0] == 'X' && field[i][1] == 'X' && field[i][2] == 'X') ||
            (field[0][i] == 'X' && field[1][i] == 'X' && field[2][i] == 'X'))
        {
            return 1;
        }
        if ((field[i][0] == 'O' && field[i][1] == 'O' && field[i][2] == 'O') ||
            (field[0][i] == 'O' && field[1][i] == 'O' && field[2][i] == 'O'))
        {
            return 2;
        }
    }
    if ((field[0][0] == 'X' && field[1][1] == 'X' && field[2][2] == 'X') ||
        (field[0][2] == 'X' && field[1][1] == 'X' && field[2][0] == 'X'))
    {
        return 1;
    }
    if ((field[0][0] == 'O' && field[1][1] == 'O' && field[2][2] == 'O') ||
        (field[0][2] == 'O' && field[1][1] == 'O' && field[2][0] == 'O'))
    {
        return 2;
    }
    return 0;
}
int botrecursion(char field[3][3], bool turn, int depth)
{
    // make bot better by checking for force-wins! linear checking for 2 possibilities to win should be fine
    char predict[3][3];
    for (int i = 0; i < 3; i++) // copy field into predict
    {
        for (int j = 0; j < 3; j++)
        {
            predict[i][j] = field[i][j];
        }
    }
    int points = 0;
    bool check = false;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (predict[j][i] == '\0')
            {
                check = true;
                if (!turn)
                    predict[j][i] = 'X';
                else
                    predict[j][i] = 'O';
            }
            if (wincheck(predict) == 0 && depth != 0)
            {
                points += botrecursion(predict, !turn, depth - 1);
            }
            else if (wincheck(predict) == 1)
                points -= depth * depth + 1;
            else if (wincheck(predict) == 2)
                points += depth * depth + 1;
            if (check)
            {
                predict[j][i] = '\0';
                check = false;
            }
        }
    }
    return points;
}
void bot(char field[3][3])
{
    int points[3][3] = {0};
    char predict[3][3];
    for (int i = 0; i < 3; i++) // copy field into predict
    {
        for (int j = 0; j < 3; j++)
        {
            predict[i][j] = field[i][j];
        }
    }
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (predict[j][i] == '\0')
            {
                predict[j][i] = 'O';
                points[j][i] = botrecursion(predict, false, 4);
                predict[j][i] = '\0';
            }
        }
    }
    int savebiggest[2];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (field[j][i] == '\0')
            {
                savebiggest[0] = j;
                savebiggest[1] = i;
            }
        }
    }
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (points[j][i] > points[savebiggest[0]][savebiggest[1]] && field[j][i] == '\0')
            {
                savebiggest[0] = j;
                savebiggest[1] = i;
            }
        }
    }
    field[savebiggest[0]][savebiggest[1]] = 'O';
    float *frame = buffer.data;
    int x_field[3] = {0, buffer.width / 3 + 1, buffer.width - buffer.width / 3};
    int y_field[3] = {0, buffer.height / 3 + 1, buffer.height - buffer.height / 3};

    for (int i = (x_field[savebiggest[0]] + 1), j = (y_field[savebiggest[1]] + 1) * buffer.height, count = 0; count < 9; count++, i = (count % 3 == 0) ? i - 2 : i + 1, j += (count % 3 == 0) ? buffer.height : 0)
    {
        frame[i * 3 + j * 3 + 2] = (count % 2) ? 1.0 : 0.0;
    }
}
void pixel_init(void)
{
    for (int i = 0; i < (buffer.width * buffer.height * 3); i++)
    {
        ((float *)(buffer.data))[i] = 0.0;
    }
}

void playfield_draw(void)
{
    pixel_init();
    float p0[] = {buffer.width / 3, 0}, p1[] = {buffer.width / 3, buffer.height}, color[] = {1.0f, 1.0f, 1.0f};
    m_raster_line(buffer.data, buffer.width, buffer.height, 3, p0, p1, color);
    p0[0] = buffer.width - p0[0] - 1;
    p1[0] = p0[0];
    m_raster_line(buffer.data, buffer.width, buffer.height, 3, p0, p1, color);
    p0[0] = 0;
    p1[0] = buffer.width;
    p0[1] = buffer.height / 3;
    p1[1] = p0[1];
    m_raster_line(buffer.data, buffer.width, buffer.height, 3, p0, p1, color);
    p0[1] = buffer.width - p0[1] - 1;
    p1[1] = p0[1];
    m_raster_line(buffer.data, buffer.width, buffer.height, 3, p0, p1, color);
}

void ctoy_begin(void)
{
    ctoy_buffer_size(1024, 1024);

    m_image_create(&buffer, M_FLOAT, PIXEL, PIXEL, 3);
    m_image_load_float(&font_default, "data/font/default.tga");
    m_image_load_float(&font_small, "data/font/small.tga");
}

void ctoy_end(void)
{
    m_image_destroy(&buffer);
}

void ctoy_main_loop(void)
{
    static int start = 0;
    static int test = 0;
    if (start == 0)
    {
        test = 0;
        pixel_init();
        float p0[] = {buffer.width - 1, 0}, p1[] = {0, buffer.height - 1}, color[] = {1.0f, 1.0f, 1.0f};
        m_raster_line(buffer.data, buffer.width, buffer.height, 3, p0, p1, color);
        int x = ctoy_mouse_x() * buffer.width;
        int y = ctoy_mouse_y() * buffer.height;
        m_font_draw(&buffer, &font_small, "B", strlen("B"), 0, 0, color);
        m_font_draw(&buffer, &font_small, "P", strlen("P"), buffer.width / 2 + 2, 0, color);
        if (ctoy_mouse_button_press(CTOY_MOUSE_BUTTON_LEFT))
        {
            if (x + y <= (buffer.width + buffer.height) / 2)
                withbot = 1;
            else
                withbot = 0;
            start = 1;
            playfield_draw();
        }
    }
    else
    {
        static int repeats = 0;
        static char field[3][3] = {{'\0'}};
        static short playerturn = 0; // if it's the turn of player 1 or 2
        float color[3] = {0, 1.0, 0};
        static int winner = 0;
        if (winner == 0 && repeats < 9)
        {
            float *image = (float *)buffer.data;
            int x = ctoy_mouse_x() * buffer.width; // muessen no mit width und height
            int y = ctoy_mouse_y() * buffer.height;
            if (withbot == 1 && playerturn == 1)
            {
                bot(field);
                playerturn = !playerturn;
                repeats++;
            }
            if (ctoy_mouse_button_press(CTOY_MOUSE_BUTTON_LEFT))
            {
                int x_pos = 0;
                int y_pos = 0;
                int x_field[3] = {0, buffer.width / 3 + 1, buffer.width - buffer.width / 3};
                int y_field[3] = {0, buffer.height / 3 + 1, buffer.height - buffer.height / 3};

                if (x > buffer.width / 3 && x < buffer.width - 1 - buffer.width / 3)
                {
                    x_pos = 1;
                }
                else if (x > buffer.width - 1 - buffer.width / 3)
                {
                    x_pos = 2;
                }
                else if (x > buffer.width / 3 - 1)
                {
                    return;
                }
                if (y > buffer.height / 3 && y < buffer.height - 1 - buffer.height / 3)
                {
                    y_pos = 1;
                }
                else if (y > buffer.height - 1 - buffer.height / 3)
                {
                    y_pos = 2;
                }
                else if (y > buffer.height / 3 - 1)
                {
                    return;
                }
                // x_field[x_pos]; x position links oben im Feld von Mausklick
                /*if(x < (buffer.width-2)/3 && y < (buffer.height-2)/3){
                    x_sign = 2;
                    y_sign = 2;
                    float *pixel = (float *) buffer.data + (y_sign * buffer.width + x_sign) * buffer.comp;
                    pixel[0] = 1.0;
                    pixel[1] = 0.0;
                    pixel[2] = 0.0;
                }*/
                if (field[x_pos][y_pos] == '\0')
                {
                    repeats++;
                    if (playerturn == 0)
                    {
                        field[x_pos][y_pos] = 'X';
                    }
                    else
                    {
                        field[x_pos][y_pos] = 'O';
                    }
                    for (int i = (x_field[x_pos] + 1), j = (y_field[y_pos] + 1) * buffer.height, count = 0; count < 9; count++, i = (count % 3 == 0) ? i - 2 : i + 1, j += (count % 3 == 0) ? buffer.height : 0)
                    {
                        if (playerturn == 0)
                        {
                            image[i * 3 + j * 3] = (count % 2) ? 0.0 : 1.0;
                        }
                        else
                        {
                            image[i * 3 + j * 3 + 2] = (count % 2) ? 1.0 : 0.0;
                        }
                        // image[(i%3) * 3 * j + 2]  = 1.0;
                    }
                    playerturn = !playerturn;
                }
            }
            winner = wincheck(field);
        }
        else if (winner == 1)
        {
            pixel_init();
            m_font_draw(&buffer, &font_default, "X", strlen("X"), 0, 0, color);
        }
        else if (winner == 2)
        {
            pixel_init();
            m_font_draw(&buffer, &font_default, "O", strlen("O"), 0, 0, color);
        }
        else
        {
            pixel_init();
            m_font_draw(&buffer, &font_default, "=", strlen("="), 0, 0, color);
        }
        if (repeats > 8 || winner != 0)
        {
            // sleep(1);
            if (ctoy_mouse_button_press(CTOY_MOUSE_BUTTON_LEFT))
            {
                if (test == 1)
                {
                    repeats = 0;
                    winner = 0;
                    playerturn = 0;
                    for (int i = 0; i < 9; i++)
                    {
                        field[i % 3][i / 3] = '\0';
                    }
                    start = 0;
                }
                test = 1;
            }
        }
    }
    ctoy_swap_buffer(&buffer);
}
