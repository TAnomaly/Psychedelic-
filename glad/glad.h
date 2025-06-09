#ifndef __glad_h_
#define __glad_h_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#define APIENTRY __stdcall
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

    typedef void *(*GLADloadproc)(const char *name);

#ifndef GLAPI
#if defined(GLAD_GLAPI_EXPORT)
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(GLAD_GLAPI_EXPORT_BUILD)
#if defined(__GNUC__)
#define GLAPI __attribute__((dllexport)) extern
#else
#define GLAPI __declspec(dllexport) extern
#endif
#else
#if defined(__GNUC__)
#define GLAPI __attribute__((dllimport)) extern
#else
#define GLAPI __declspec(dllimport) extern
#endif
#endif
#elif defined(__GNUC__) && defined(GLAD_GLAPI_EXPORT_BUILD)
#define GLAPI __attribute__((visibility("default"))) extern
#else
#define GLAPI extern
#endif
#else
#define GLAPI extern
#endif
#endif

    typedef void GLvoid;
    typedef unsigned int GLenum;
    typedef float GLfloat;
    typedef int GLint;
    typedef int GLsizei;
    typedef unsigned int GLbitfield;
    typedef double GLdouble;
    typedef unsigned int GLuint;
    typedef unsigned char GLboolean;
    typedef unsigned char GLubyte;
    typedef char GLchar;
    typedef short GLshort;
    typedef signed char GLbyte;
    typedef unsigned short GLushort;
    typedef unsigned long GLulong;
    typedef unsigned short GLhalf;
    typedef long GLfixed;
    typedef void *GLsync;
    typedef void *GLintptr;
    typedef void *GLsizeiptr;
    typedef int GLsizei;

#define GL_FALSE 0
#define GL_TRUE 1

// OpenGL ES Standard Attributes
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_NONE 0
#define GL_FRONT_LEFT 0x0400
#define GL_FRONT_RIGHT 0x0401
#define GL_BACK_LEFT 0x0402
#define GL_BACK_RIGHT 0x0403
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_LEFT 0x0406
#define GL_RIGHT 0x0407
#define GL_FRONT_AND_BACK 0x0408
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY 0x0505
#define GL_CW 0x0900
#define GL_CCW 0x0901
#define GL_POINT_SIZE 0x0B11
#define GL_POINT_SIZE_RANGE 0x0B12
#define GL_POINT_SIZE_GRANULARITY 0x0B13
#define GL_LINE_SMOOTH 0x0B20
#define GL_LINE_WIDTH 0x0B21
#define GL_LINE_WIDTH_RANGE 0x0B22
#define GL_LINE_WIDTH_GRANULARITY 0x0B23
#define GL_POLYGON_MODE 0x0B40
#define GL_POLYGON_SMOOTH 0x0B41
#define GL_CULL_FACE 0x0B44
#define GL_CULL_FACE_MODE 0x0B45
#define GL_FRONT_FACE 0x0B46
#define GL_DEPTH_RANGE 0x0B70
#define GL_DEPTH_TEST 0x0B71
#define GL_DEPTH_WRITEMASK 0x0B72
#define GL_DEPTH_CLEAR_VALUE 0x0B73
#define GL_DEPTH_FUNC 0x0B74
#define GL_STENCIL_TEST 0x0B90
#define GL_STENCIL_CLEAR_VALUE 0x0B91
#define GL_STENCIL_FUNC 0x0B92
#define GL_STENCIL_VALUE_MASK 0x0B93
#define GL_STENCIL_FAIL 0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS 0x0B96
#define GL_STENCIL_REF 0x0B97
#define GL_STENCIL_WRITEMASK 0x0B98
#define GL_VIEWPORT 0x0BA2
#define GL_DITHER 0x0BD0
#define GL_BLEND_DST 0x0BE0
#define GL_BLEND_SRC 0x0BE1
#define GL_BLEND 0x0BE2
#define GL_LOGIC_OP_MODE 0x0BF0
#define GL_COLOR_LOGIC_OP 0x0BF2
#define GL_DRAW_BUFFER 0x0C01
#define GL_READ_BUFFER 0x0C02
#define GL_SCISSOR_BOX 0x0C10
#define GL_SCISSOR_TEST 0x0C11
#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_COLOR_WRITEMASK 0x0C23
#define GL_DOUBLEBUFFER 0x0C32
#define GL_STEREO 0x0C33
#define GL_LINE_SMOOTH_HINT 0x0C52
#define GL_POLYGON_SMOOTH_HINT 0x0C53
#define GL_UNPACK_SWAP_BYTES 0x0CF0
#define GL_UNPACK_LSB_FIRST 0x0CF1
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_PACK_SWAP_BYTES 0x0D00
#define GL_PACK_LSB_FIRST 0x0D01
#define GL_PACK_ROW_LENGTH 0x0D02

    typedef void(APIENTRY *PFNGLCLEARPROC)(GLbitfield mask);
    typedef void(APIENTRY *PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    typedef void(APIENTRY *PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
    typedef void(APIENTRY *PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
    typedef void(APIENTRY *PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
    typedef void(APIENTRY *PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
    typedef void(APIENTRY *PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
    typedef void(APIENTRY *PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    typedef void(APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
    typedef void(APIENTRY *PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
    typedef void(APIENTRY *PFNGLBINDVERTEXARRAYPROC)(GLuint array);
    typedef void(APIENTRY *PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
    typedef void(APIENTRY *PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
    typedef void(APIENTRY *PFNGLCREATESHADERPROC)(GLenum type);
    typedef void(APIENTRY *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length);
    typedef void(APIENTRY *PFNGLCOMPILESHADERPROC)(GLuint shader);
    typedef void(APIENTRY *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
    typedef void(APIENTRY *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    typedef void(APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
    typedef void(APIENTRY *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
    typedef void(APIENTRY *PFNGLLINKPROGRAMPROC)(GLuint program);
    typedef void(APIENTRY *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
    typedef void(APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    typedef void(APIENTRY *PFNGLDELETESHADERPROC)(GLuint shader);
    typedef void(APIENTRY *PFNGLDELETEPROGRAMPROC)(GLuint program);
    typedef void(APIENTRY *PFNGLUSEPROGRAMPROC)(GLuint program);
    typedef GLint(APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);
    typedef void(APIENTRY *PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
    typedef void(APIENTRY *PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0, GLfloat v1);
    typedef void(APIENTRY *PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    typedef void(APIENTRY *PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    typedef void(APIENTRY *PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
    typedef const GLubyte *(APIENTRY *PFNGLGETSTRINGPROC)(GLenum name);
    typedef void(APIENTRY *PFNGLACTIVETEXTUREPROC)(GLenum texture);
    typedef void(APIENTRY *PFNGLGENTEXTURESPROC)(GLsizei n, GLuint *textures);
    typedef void(APIENTRY *PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
    typedef void(APIENTRY *PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
    typedef void(APIENTRY *PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
    typedef void(APIENTRY *PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint *textures);

    GLAPI PFNGLCLEARPROC glad_glClear;
#define glClear glad_glClear
    GLAPI PFNGLCLEARCOLORPROC glad_glClearColor;
#define glClearColor glad_glClearColor
    GLAPI PFNGLDRAWARRAYSPROC glad_glDrawArrays;
#define glDrawArrays glad_glDrawArrays
    GLAPI PFNGLVIEWPORTPROC glad_glViewport;
#define glViewport glad_glViewport
    GLAPI PFNGLGENBUFFERSPROC glad_glGenBuffers;
#define glGenBuffers glad_glGenBuffers
    GLAPI PFNGLBINDBUFFERPROC glad_glBindBuffer;
#define glBindBuffer glad_glBindBuffer
    GLAPI PFNGLBUFFERDATAPROC glad_glBufferData;
#define glBufferData glad_glBufferData
    GLAPI PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer;
#define glVertexAttribPointer glad_glVertexAttribPointer
    GLAPI PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
#define glEnableVertexAttribArray glad_glEnableVertexAttribArray
    GLAPI PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays;
#define glGenVertexArrays glad_glGenVertexArrays
    GLAPI PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray;
#define glBindVertexArray glad_glBindVertexArray
    GLAPI PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays;
#define glDeleteVertexArrays glad_glDeleteVertexArrays
    GLAPI PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers;
#define glDeleteBuffers glad_glDeleteBuffers
    GLAPI PFNGLCREATESHADERPROC glad_glCreateShader;
#define glCreateShader glad_glCreateShader
    GLAPI PFNGLSHADERSOURCEPROC glad_glShaderSource;
#define glShaderSource glad_glShaderSource
    GLAPI PFNGLCOMPILESHADERPROC glad_glCompileShader;
#define glCompileShader glad_glCompileShader
    GLAPI PFNGLGETSHADERIVPROC glad_glGetShaderiv;
#define glGetShaderiv glad_glGetShaderiv
    GLAPI PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog;
#define glGetShaderInfoLog glad_glGetShaderInfoLog
    GLAPI PFNGLCREATEPROGRAMPROC glad_glCreateProgram;
#define glCreateProgram glad_glCreateProgram
    GLAPI PFNGLATTACHSHADERPROC glad_glAttachShader;
#define glAttachShader glad_glAttachShader
    GLAPI PFNGLLINKPROGRAMPROC glad_glLinkProgram;
#define glLinkProgram glad_glLinkProgram
    GLAPI PFNGLGETPROGRAMIVPROC glad_glGetProgramiv;
#define glGetProgramiv glad_glGetProgramiv
    GLAPI PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog;
#define glGetProgramInfoLog glad_glGetProgramInfoLog
    GLAPI PFNGLDELETESHADERPROC glad_glDeleteShader;
#define glDeleteShader glad_glDeleteShader
    GLAPI PFNGLDELETEPROGRAMPROC glad_glDeleteProgram;
#define glDeleteProgram glad_glDeleteProgram
    GLAPI PFNGLUSEPROGRAMPROC glad_glUseProgram;
#define glUseProgram glad_glUseProgram
    GLAPI PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation;
#define glGetUniformLocation glad_glGetUniformLocation
    GLAPI PFNGLUNIFORM1FPROC glad_glUniform1f;
#define glUniform1f glad_glUniform1f
    GLAPI PFNGLUNIFORM2FPROC glad_glUniform2f;
#define glUniform2f glad_glUniform2f
    GLAPI PFNGLUNIFORM3FPROC glad_glUniform3f;
#define glUniform3f glad_glUniform3f
    GLAPI PFNGLUNIFORM4FPROC glad_glUniform4f;
#define glUniform4f glad_glUniform4f
    GLAPI PFNGLUNIFORM1IPROC glad_glUniform1i;
#define glUniform1i glad_glUniform1i
    GLAPI PFNGLGETSTRINGPROC glad_glGetString;
#define glGetString glad_glGetString
    GLAPI PFNGLACTIVETEXTUREPROC glad_glActiveTexture;
#define glActiveTexture glad_glActiveTexture
    GLAPI PFNGLGENTEXTURESPROC glad_glGenTextures;
#define glGenTextures glad_glGenTextures
    GLAPI PFNGLBINDTEXTUREPROC glad_glBindTexture;
#define glBindTexture glad_glBindTexture
    GLAPI PFNGLTEXIMAGE2DPROC glad_glTexImage2D;
#define glTexImage2D glad_glTexImage2D
    GLAPI PFNGLTEXPARAMETERIPROC glad_glTexParameteri;
#define glTexParameteri glad_glTexParameteri
    GLAPI PFNGLDELETETEXTURESPROC glad_glDeleteTextures;
#define glDeleteTextures glad_glDeleteTextures

    GLAPI int gladLoadGL(void);
    GLAPI int gladLoadGLLoader(GLADloadproc);

    struct gladGLversionStruct
    {
        int major;
        int minor;
    };

    extern struct gladGLversionStruct GLVersion;

#ifdef __cplusplus
}
#endif

#endif