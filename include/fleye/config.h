#ifndef __fleye_config_h
#define __fleye_config_h

#define PROCESSING_GPU		    		-1
#define PROCESSING_MAIN_THREAD		    0
#define PROCESSING_ASYNC_THREADS		2

#define IMGPROC_MAX_CPU_FUNCS 16

#define MAX_POINT_SIZE 512

//#ifndef NDEBUG
#define CHECK_GL_ERRORS 1
//#endif

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


