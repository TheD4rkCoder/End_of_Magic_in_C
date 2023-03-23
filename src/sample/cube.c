#include <ctoy.h>

struct m_image image = M_IMAGE_IDENTITY();

// a list of 8 vertices in the shape of a cube
float3 vertices[8] = {
    {-1, -1, -1},
    { 1, -1, -1},
    { 1,  1, -1},
    {-1,  1, -1},
    {-1, -1,  1},
    { 1, -1,  1},
    { 1,  1,  1},
    {-1,  1,  1}
};

void ctoy_begin(void)
{
    // create a 256 x 256 RGB image
    m_image_create(&image, M_FLOAT, 256, 256, 3);

    // clear the image to zero
    memset(image.data, 0, image.size * sizeof(float));
}

void ctoy_end(void)
{
    // free the image
    m_image_destroy(&image);
}

void ctoy_main_loop(void)
{
    float3 tmp_vertices[8];
    float3 pos = {0, 0, 5}; // we want to move the cube in front of the camera
    int i;

    float matrix[16] = M_MAT4_IDENTITY(); // let's declare a proper matrix
    float t = (ctoy_t() % 200) / 200.0f;  // get ctoy's tick to animate all this
    float3 rot = {                        // declare x, y, z Euler rotation
        t * M_PI * 2,
        t * M_PI * 2,
        0
    };

    m_mat4_rotation_euler(matrix, &rot); // create the rotation matrix
    m_mat4_translation(matrix, &pos);    // set the matrix translation

    // transformation
    for (i = 0; i < 8; i++)
        m_mat4_transform3(&tmp_vertices[i], matrix, &vertices[i]);

    // simple perspective projection
    for (i = 0; i < 8; i++)
    {
        // we divide x and y coordinates by z coordinate and center the result
        tmp_vertices[i].x = (tmp_vertices[i].x / tmp_vertices[i].z) + 0.5f;
        tmp_vertices[i].y = (tmp_vertices[i].y / tmp_vertices[i].z) + 0.5f;
    }

    for (i = 0; i < image.size; i++) // fade to zero instead of clearing to zero
        ((float *)image.data)[i] *= 0.8f;

    // clear the image to zero
    //memset(image.data, 0, image.size * sizeof(float));

    // draw points
    for (i = 0; i < 8; i++)
    {
        // get the pixel coordinates
        int x = tmp_vertices[i].x * image.width;
        int y = tmp_vertices[i].y * image.height;

        // if the pixel is not out of range, draw a point
        if (x >= 0 && y >= 0 && x < image.width && y < image.height)
        {
            float *pixel = (float *)image.data + (y * image.width + x) * image.comp;
            // pixel[0] = 1.0f; // R
            // pixel[1] = 1.0f; // G
            // pixel[2] = 1.0f; // B

            // add instead of setting
            pixel[0] += 0.5f;
            pixel[1] += 1.0f;
            pixel[2] += 0.5f;
        }
    }

    // blur !
    m_image_gaussian_blur(&image, &image, 1.5, 1.5);

    // send the image to the screen
    ctoy_swap_buffer(&image);

    t++;
}