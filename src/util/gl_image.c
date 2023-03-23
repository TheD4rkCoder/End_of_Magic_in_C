#include "gl_utils.c"


static struct m_image glim_ubyte = M_IMAGE_IDENTITY();
static int glim_texture;
static int glim_width = 0;
static int glim_height = 0;

static GLuint glim_vert_shader;
static GLuint glim_frag_shader;
static GLuint glim_prog_object;
static int glim_uTexSize;
static int glim_uScreenSize;
static int glim_uTime;

static char glim_vert_src[] =
"attribute vec2 aVertex;\n"
"varying vec2 vTexcoord;"
"void main()\n"
"{\n"
" vec2 p = aVertex.xy - 0.5;"
" vTexcoord = vec2(aVertex.x, 1.0-aVertex.y);"
" gl_Position = vec4(p.x*2.0, p.y*2.0, 0.0, 1.0); \n"
"}\n";                             

static char glim_frag_src[] =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"#define M_PI 3.14159265358979323846\n"
"uniform sampler2D uTexture0;"
"uniform vec2 uTexSize;"
"uniform vec2 uScreenSize;"
"uniform float uTime;"

"varying vec2 vTexcoord;"


"float nrand(vec2 n) {"
"  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);"
"}"


"vec4 tolinear(vec4 v) {"
" return vec4(pow(v.x, 2.2), pow(v.y, 2.2), pow(v.z, 2.2), pow(v.w, 2.2));" 
"}"


"vec4 tosrgb(vec4 v) {"
" return vec4(pow(v.x, 1.0/2.2), pow(v.y, 1.0/2.2), pow(v.z, 1.0/2.2), pow(v.w, 1.0/2.2));" 
"}"


"vec4 sample(vec2 texcoord, float yforce)"
"{"
" vec2 dp = vec2(1.0, 1.0) / uTexSize;"

" float cosx = 1.0 - abs(cos(texcoord.x * (uTexSize.x) * M_PI * 1.0));"
" float cosy = 1.0 - abs(cos(texcoord.y * (uTexSize.y) * M_PI));"
" float y = cosy * (cosx*0.15 + 0.85);"

" vec2 texcoif = texcoord * uTexSize;"
" vec2 texcoi = floor(texcoif);"
" vec2 mu = (texcoif - 0.5) - texcoi;"

" vec2 mub;"
" mub.x = pow(abs(mu.x) * 2.0, 2.5);" // 4.5
" mub.y = pow(abs(mu.y) * 2.0, 6.0);"
" mub *= sign(mu) * 0.5;"

" vec2 texco = (texcoi + mub + 0.5) / uTexSize;"

" vec4 texColor1 = tolinear(texture2D(uTexture0, texco));"
" vec4 color = texColor1 * ((1.0 - yforce) + y * yforce);"

" return color;\n"
"}"

"void main()"
"{"

" vec4 color, blur;"
" vec2 v = vec2(1.0) / uScreenSize;"
" vec2 coord = vTexcoord;"
" vec2 coordv = coord - 0.5;"
" float yforce = 0.75;" // 0.65

" float tf = fract(uTime*0.0001);"
" float r1 = nrand(vTexcoord + tf + vec2(0.0, 1.0));"

" float vign = length(coordv);"

" coord = (coordv + coordv * vign* 0.025) * 0.99;"

" vec2 vign2 = clamp(abs(coord) * 2.0, 0.0, 1.0);"

" coord += 0.5;"

" color =   sample(coord + vec2(v.x*0.250, v.y*0.125), yforce);"
" color += sample(coord + vec2(v.x*0.875, v.y*0.250), yforce);"
" color += sample(coord + vec2(v.x*0.875, v.y*0.750), yforce);"
" color += sample(coord + vec2(v.x*0.125, v.y*0.750), yforce);"
" color *= 0.3;"

" blur = tolinear(texture2D(uTexture0, coord)) * 0.99 + 0.01;"

" float vf = pow(1.0 - vign, 2.5);"
" color *= 1.0 + vf * 1.5;"
" color += blur * vf * 0.35;"

" color *= 1.0 - pow(vign2.x, 20.0)*0.5;"
" color *= 1.0 - pow(vign2.y, 20.0)*0.5;"

" gl_FragColor = tosrgb(color + (r1 - 0.5)*(0.75 / 255.0));\n"
"}";


static void glim_texture_size(int width, int height)
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glim_width = width;
    glim_height = height;
}

static void glim_update(struct m_image *image)
{
    glBindTexture(GL_TEXTURE_2D, glim_texture);

    if (image->width != glim_width || image->height != glim_height)
        glim_texture_size(image->width, image->height);

    if (image->type == M_FLOAT)
    {
        m_image_float_to_srgb(&glim_ubyte, image);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, glim_ubyte.width, glim_ubyte.height, GL_RGB, GL_UNSIGNED_BYTE, glim_ubyte.data);
    }
    else if (image->type == M_UBYTE)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image->width, image->height, GL_RGB, GL_UNSIGNED_BYTE, image->data);
    }
}

static void glim_draw(void)
{
    static float vertices[8] = {0, 0, 0, 1, 1, 0, 1, 1};

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void glim_begin(void)
{
    int uTexture;

    glGenTextures(1, &glim_texture);
    glBindTexture(GL_TEXTURE_2D, glim_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glim_vert_shader = load_shader(GL_VERTEX_SHADER, glim_vert_src);
    glim_frag_shader = load_shader(GL_FRAGMENT_SHADER, glim_frag_src);

    glim_prog_object = glCreateProgram();
    if (glim_prog_object == 0)
        return;

    glAttachShader(glim_prog_object, glim_vert_shader);
    glAttachShader(glim_prog_object, glim_frag_shader);

    glBindAttribLocation(glim_prog_object, 0, "aVertex");
    glLinkProgram(glim_prog_object);

    glUseProgram(glim_prog_object);

    uTexture = glGetUniformLocation(glim_prog_object, "uTexture0");
    glUniform1i(uTexture, 0);
    glim_uTexSize = glGetUniformLocation(glim_prog_object, "uTexSize");
    glim_uScreenSize = glGetUniformLocation(glim_prog_object, "uScreenSize");
	glim_uTime = glGetUniformLocation(glim_prog_object, "uTime");
}

void glim_main_loop(void)
{
    glClearColor(0, 0.2, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glViewport(0, 0, ctoy_window_width(), ctoy_window_height());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glim_texture);
    glUseProgram(glim_prog_object);
    glUniform2f(glim_uTexSize, (float)glim_width, (float)glim_height);
    glUniform2f(glim_uScreenSize, (float)ctoy_window_width(), (float)ctoy_window_height());
	glUniform1f(glim_uTime, (float)ctoy_t());
	
    glim_draw();
    
    glUseProgram(0);

    ctoy_swap_buffer(0);
}

void glim_end(void)
{
    glDeleteTextures(1, &glim_texture);
    m_image_destroy(&glim_ubyte);
}
