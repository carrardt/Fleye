#ifndef __fleye_config_h
#define __fleye_config_h

#define RASPITEX_VERSION_MAJOR 1
#define RASPITEX_VERSION_MINOR 0

#define MAX_TEXTURES 16
#define MAX_FBOS 16

#define SHADER_MAX_INPUT_TEXTURES 4
#define SHADER_MAX_OUTPUT_FBOS 4
#define UNIFORM_NAME_MAX_LEN 64
#define TEXTURE_NAME_MAX_LEN 64

#define SHADER_MAX_ATTRIBUTES 16
#define SHADER_MAX_UNIFORMS   16

#define SHADER_COMPILE_CACHE_SIZE 16
#define IMGPROC_MAX_STEPS 16

#define PROCESSING_GPU		    		-1
#define PROCESSING_MAIN_THREAD		    0
#define PROCESSING_ASYNC_THREAD			1

#define MAX_OPT_VALUES 16

#define MAX_TRACKED_OBJECTS 8
#define IMGPROC_MAX_CPU_FUNCS 16

#define MAX_POINT_SIZE 512

#ifndef NDEBUG
#define CHECK_GL_ERRORS 1
#endif

#if defined(CHECK_GL_ERRORS)
#include <stdio.h>
#include <assert.h>
#define GLCHK(X) \
do { \
    GLenum __fleye_gl_err = GL_NO_ERROR; \
    X; \
   while ((__fleye_gl_err = glGetError())) \
   { \
      fprintf(stderr,"GL error 0x%x in " #X "file %s line %d\n", __fleye_gl_err, __FILE__,__LINE__); \
      assert(__fleye_gl_err == GL_NO_ERROR); \
   } \
} \
while(0)
#else
#define GLCHK(X) X
#endif /* CHECK_GL_ERRORS */


#endif

