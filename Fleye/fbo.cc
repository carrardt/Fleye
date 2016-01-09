#include <GLES2/gl2.h>
#include <string.h>

#include "fleye/fbo.h"
#include "fleye/imageprocessing.h"
#include "fleye/texture.h"

#include <iostream>

FrameBufferObject* add_fbo(ImageProcessingState* ip, const std::string& name, GLint colorFormat, GLint w, GLint h, GLenum filtering)
{
	// create frame buffer object storage
	FrameBufferObject* fbo = new FrameBufferObject;
	fbo->width = w;
	fbo->height = h;
	fbo->fb = 0;
	glGenFramebuffers(1, & fbo->fb);

	// create associtated texture
	fbo->texture = new GLTexture;
	fbo->texture->format = colorFormat;
	fbo->texture->target = GL_TEXTURE_2D;
	fbo->texture->texid = 0;
	glGenTextures(1, & fbo->texture->texid );

	// regiter fbo and texture with the same name
	ip->fbo[name] = fbo;
	ip->texture[name] = fbo->texture;

	glBindTexture(fbo->texture->target, fbo->texture->texid);
	glTexImage2D(fbo->texture->target, 0, fbo->texture->format, fbo->width, fbo->height, 0, fbo->texture->format/*GL_RGBA*/, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(fbo->texture->target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(fbo->texture->target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(fbo->texture->target, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(fbo->texture->target, GL_TEXTURE_MAG_FILTER, filtering);
	glBindTexture(fbo->texture->target, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fb);

    // Associate the textures with the FBO.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    fbo->texture->target, fbo->texture->texid, 0);

	// no depth buffer
	//TODO: add support for depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                    GL_TEXTURE_2D, /*null texture object*/ 0, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	
    if ( status == GL_FRAMEBUFFER_COMPLETE )
    {
		return fbo;
	}
    else 
    {
		return NULL;
	}
}
