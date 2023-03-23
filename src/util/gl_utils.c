#include <GLES2/gl2.h>


static GLuint load_shader(GLenum type, const char *src)
{
    GLuint shader;
    GLint compiled;

    shader = glCreateShader(type);
    if (shader == 0)
        return 0;

    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {

        GLint ilen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ilen);
        if (ilen > 1)
        {
            char *infolog = malloc(sizeof(char) * ilen);
            glGetShaderInfoLog(shader, ilen, NULL, infolog);
            printf("Error compiling shader:\n%s\n", infolog);
            free(infolog);
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}
