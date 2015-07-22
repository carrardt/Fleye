/*
Copyright (c) 2013, Broadcom Europe Ltd
Copyright (c) 2013, Tim Gover
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//#define CHECK_GL_ERRORS 1

#include "RaspiTex.h"
#include "RaspiTexUtil.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "cpu_tracking.h"

static GLfloat varray[8] =
{
   0.0f, 0.0f
};

static const EGLint tracking_egl_config_attribs[] =
{
   EGL_RED_SIZE,   8,
   EGL_GREEN_SIZE, 8,
   EGL_BLUE_SIZE,  8,
   EGL_ALPHA_SIZE, 8,
   //EGL_SURFACE_TYPE , EGL_LOCK_SURFACE_BIT_KHR,
   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
   EGL_NONE
};

static int tracking_init(RASPITEX_STATE *state)
{	
	RASPITEX_FBO dispWinFBO;
   int i,rc;
    state->egl_config_attribs = tracking_egl_config_attribs;
    rc = raspitexutil_gl_init_2_0(state);
    if (rc != 0)
    {
		vcos_log_error("unable to init GLES2\n");
		return rc;
	}

	{ char tmp[64]; sprintf(tmp,"%d",state->width); raspitex_add_optional_value(state,"WIDTH",tmp); }
	{ char tmp[64]; sprintf(tmp,"%d",state->height); raspitex_add_optional_value(state,"HEIGHT",tmp); }

	// configure camera input texture
   GLCHK( glBindTexture(GL_TEXTURE_EXTERNAL_OES, state->texture) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
   GLCHK( glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
   GLCHK( glBindTexture(GL_TEXTURE_EXTERNAL_OES,0) );
   
   // define input image as an available input texture named "INPUT"
	state->processing_texture[0].texid = state->texture;
	state->processing_texture[0].target = GL_TEXTURE_EXTERNAL_OES;
	state->processing_texture[0].format = GL_RGB;
	strcpy(state->processing_texture[0].name,"CAMERA");

	// define display window an available output named "DISPLAY"
	// define null texture associated with final display (not a real FBO)
	state->processing_texture[1].texid = 0;
	state->processing_texture[1].target = GL_TEXTURE_2D;
	state->processing_texture[1].format = GL_RGB;
	strcpy(state->processing_texture[1].name,"DISPLAY");
	state->processing_fbo[0].texture = & state->processing_texture[1];
	state->processing_fbo[0].width = state->width;
	state->processing_fbo[0].height = state->height;
	state->processing_fbo[0].fb = 0;

	state->nFBO = 1;
	state->nTextures = 2;

	create_image_processing( state, state->tracking_script );

	GLCHK( glDisable(GL_DEPTH_TEST) );
	
	// allocate space for CPU processing
	
	printf( "EGL_CLIENT_APIS: %s\n", eglQueryString( state->display, EGL_CLIENT_APIS ) );
	printf( "EGL_VENDOR: %s\n", eglQueryString( state->display, EGL_VENDOR ) );
	printf( "EGL_VERSION: %s\n", eglQueryString( state->display, EGL_VERSION ) );
	printf( "EGL_EXTENSIONS: %s\n", eglQueryString( state->display, EGL_EXTENSIONS ) );
	printf( "GL_VERSION: %s\n", glGetString( GL_VERSION ) );
	printf( "GL_SHADING_LANGUAGE_VERSION: %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
	printf( "GL_EXTENSIONS: %s\n", glGetString( GL_EXTENSIONS ) );

	state->cpu_tracking_state.width = state->width;
	state->cpu_tracking_state.height = state->height;
	state->cpu_tracking_state.image = malloc(state->cpu_tracking_state.width*state->cpu_tracking_state.height*4);
	state->cpu_tracking_state.objectCount = 0;
	state->cpu_tracking_state.do_processing = 1;
	rc = vcos_semaphore_create(& state->cpu_tracking_state.start_processing_sem,"start_processing_sem", 1);
	if (rc != VCOS_SUCCESS)
	{
		 vcos_log_error("Failed to create start_processing_sem semaphor %d",rc);
		 return rc;
	}

	rc = vcos_semaphore_create(& state->cpu_tracking_state.end_processing_sem,"end_processing_sem", 1);
	if (rc != VCOS_SUCCESS)
	{
		 vcos_log_error("Failed to create end_processing_sem semaphor %d",rc);
		 return rc;
	}

	rc = vcos_thread_create(& state->cpuTrackingThread, "cpu-tracking-worker", NULL, cpuTrackingWorker, & state->cpu_tracking_state);
	if (rc != VCOS_SUCCESS)
	{
      vcos_log_error("Failed to start cpu processing thread %d",rc);
      return rc;
	}

    return rc;
}

static CompiledShaderCache* get_compiled_shader(ShaderPass* shaderPass,RASPITEX_Texture** inputs)
{
	const char* image_external_pragma = "";
	char textureUniformProlog[1024]={'\0',};
	char* fragmentSource = 0;
	int i;
	for(i=0;i<shaderPass->compileCacheSize;i++)
	{
		int j,found=1;
		for(j=0;j<shaderPass->nInputs && found;j++)
		{
			if( shaderPass->shaderCahe[i].textureTargets[j] != inputs[j]->target ) found=0;
		}
		if(found) return & shaderPass->shaderCahe[i];
	}
	if(shaderPass->compileCacheSize>=SHADER_COMPILE_CACHE_SIZE)
	{
		vcos_log_error("Shader cache is full");
		return 0;
	}

	CompiledShaderCache* compiledShader = & shaderPass->shaderCahe[ shaderPass->compileCacheSize ++ ];
	for(i=0;i<shaderPass->nInputs;i++)
	{
		const char* samplerType = 0;
		//const char* texlookup = "texture2D";
		char uniformDeclare[128];
		switch( inputs[i]->target )
		{
			case GL_TEXTURE_2D:
				samplerType = "sampler2D";
				break;
			case GL_TEXTURE_EXTERNAL_OES:
				samplerType = "samplerExternalOES";
				image_external_pragma = "#extension GL_OES_EGL_image_external : require\n";
				break;
			default:
				vcos_log_error("unhandled texture target");
				break;
		}
		compiledShader->textureTargets[i]=inputs[i]->target;
		sprintf(uniformDeclare,"uniform %s %s;\n",samplerType,shaderPass->inputs[i].uniformName);
		strcat(textureUniformProlog,uniformDeclare);
	}
	fragmentSource = malloc( strlen(image_external_pragma) + strlen(shaderPass->fragmentSourceWithoutTextures) + strlen(textureUniformProlog) + 8 );
	sprintf(fragmentSource,"%s\n%s\n%s\n",image_external_pragma,textureUniformProlog,shaderPass->fragmentSourceWithoutTextures);

	// printf("%s FS:\n%s",shaderPass->finalTexture->name,fragmentSource);
	// compile assembled final shader program
	create_image_shader( & compiledShader->shader, shaderPass->vertexSource, fragmentSource );
	free(fragmentSource);

	for(i=0;i<shaderPass->nInputs;i++)
	{
		compiledShader->samplerUniformLocations[i] = glGetUniformLocation(compiledShader->shader.program, shaderPass->inputs[i].uniformName);
		//printf("sampler uniform '%s' -> location %d\n",shaderPass->inputs[i].uniformName,compiledShader->samplerUniformLocations[i]);
	}

	return compiledShader;
}

static void apply_shader_pass(RASPITEX_STATE *state, ShaderPass* shaderPass, int passCounter, int* needSwapBuffers)
{
	RASPITEX_Texture* inputs[MAX_TEXTURES]={0,};
	RASPITEX_FBO* destFBO=0;
	CompiledShaderCache* compiledShader=0;
	int i=0;
	GLint loc=-1;

	for(i=0;i<shaderPass->nInputs;i++)
	{
		inputs[i] = shaderPass->inputs[i].texPool[ passCounter % shaderPass->inputs[i].poolSize ];
	}
	compiledShader = get_compiled_shader( shaderPass, inputs );
	
	destFBO = shaderPass->fboPool[ passCounter % shaderPass->fboPoolSize ];

	// bind FBO and discard color buffer content (not using blending and writing everything)
    GLCHK(glBindFramebuffer(GL_FRAMEBUFFER,destFBO->fb));
	{
		GLenum attachements[1];
		if( destFBO->fb==0 ) // FBO is the native window displayed on screen
		{
			attachements[0] = GL_COLOR_EXT;
			*needSwapBuffers = 1;
		}
		else
		{
			attachements[0] = GL_COLOR_ATTACHMENT0;
		}
		GLCHK( glDiscardFramebufferEXT(GL_FRAMEBUFFER,1,attachements) );
	}

	// set viewport to full surface
    GLCHK(glViewport(0,0,destFBO->width,destFBO->height));

	// select compiled shader program to use
    GLCHK(glUseProgram(compiledShader->shader.program));

	// enable input textures
	for(i=shaderPass->nInputs-1;i>=0;i--)
	{
		if( compiledShader->samplerUniformLocations[i] != -1 )
		{
			GLCHK( glActiveTexture( GL_TEXTURE0 + i ) );
			GLCHK( glEnable(inputs[i]->target) );
			GLCHK( glBindTexture(inputs[i]->target, inputs[i]->texid) );
			GLCHK( glUniform1i(compiledShader->samplerUniformLocations[i], i) );
		}
	}

	// set uniform values
	double p2i = 1<<passCounter;
	if( (loc=compiledShader->shader.uniform_locations[0]) != -1 ) { GLCHK( glUniform2f(loc, 1.0 / destFBO->width, 1.0 / destFBO->height ) ); }
	if( (loc=compiledShader->shader.uniform_locations[1]) != -1 ) { GLCHK( glUniform2f(loc, destFBO->width, destFBO->height ) ); }
	if( (loc=compiledShader->shader.uniform_locations[2]) != -1 ) { GLCHK( glUniform1f(loc, passCounter ) ); }
	if( (loc=compiledShader->shader.uniform_locations[3]) != -1 ) { GLCHK( glUniform1f(loc, p2i ) ); }
	if( (loc=compiledShader->shader.uniform_locations[4]) != -1 ) { GLCHK( glUniform2f(loc, p2i/destFBO->width, p2i/destFBO->height ) ); }
	if( (loc=compiledShader->shader.uniform_locations[5]) != -1 ) { GLCHK( glUniform2fv(loc, 1, state->cpu_tracking_state.objectCenter[0] ) ); }
	if( (loc=compiledShader->shader.uniform_locations[6]) != -1 ) { GLCHK( glUniform2fv(loc, 1, state->cpu_tracking_state.objectCenter[1] ) ); }

	// draw geometry (a single point sprite covering entire surface)
	// TODO: make geometry customizable
    GLCHK( glEnableVertexAttribArray(compiledShader->shader.attribute_locations[0]));
    GLCHK( glVertexAttribPointer(compiledShader->shader.attribute_locations[0], 2, GL_FLOAT, GL_FALSE, 0, varray));
    GLCHK( glDrawArrays(GL_POINTS, 0, 1));
    GLCHK( glDisableVertexAttribArray(compiledShader->shader.attribute_locations[0]));

	// detach textures
 	for(i=shaderPass->nInputs-1;i>=0;i--)
	{
 		if( compiledShader->samplerUniformLocations[i] != -1 )
		{
			GLCHK( glActiveTexture( GL_TEXTURE0 + i ) );
			GLCHK( glBindTexture(inputs[i]->target,0) );
			GLCHK( glDisable(inputs[i]->target) );
		}
	}
	GLCHK( glActiveTexture( GL_TEXTURE0 ) ); // back to default

	// TODO: test if this is necessary
    GLCHK(glFinish());
}

static int tracking_redraw(RASPITEX_STATE *state)
{
	int step = 0;
	int swapBuffers = 0;

	// wait previous async cycle to be finished
	int nPrevTasksToWait = state->cpu_tracking_state.nAvailCpuFuncs - state->cpu_tracking_state.nFinishedCpuFuncs;
	//printf("waiting %d (%d/%d) previous tasks\n",nPrevTasksToWait,state->cpu_tracking_state.nFinishedCpuFuncs,state->cpu_tracking_state.nAvailCpuFuncs);
	while( nPrevTasksToWait > 0 )
	{
		vcos_semaphore_wait( & state->cpu_tracking_state.end_processing_sem );
		-- nPrevTasksToWait;
	}
	state->cpu_tracking_state.nAvailCpuFuncs = 0;
	state->cpu_tracking_state.nFinishedCpuFuncs = 0;

    //glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);
    GLCHK( glActiveTexture(GL_TEXTURE0) );

	for(step=0; step<state->nProcessingSteps; ++step)
	{
		int nPasses = state->processing_step[step].numberOfPasses;

		if ( nPasses == CPU_PROCESSING_PASS )
		{
			if( state->processing_step[step].exec_thread == 0 )
			{
				//printf("sync exec cpu step #%d\n",step);
				( * state->processing_step[step].cpu_processing ) (& state->cpu_tracking_state);
			}
			else
			{
				//printf("async start cpu step #%d\n",step);
				state->cpu_tracking_state.cpu_processing[ state->cpu_tracking_state.nAvailCpuFuncs ] = state->processing_step[step].cpu_processing;
				++ state->cpu_tracking_state.nAvailCpuFuncs;
				vcos_semaphore_post(& state->cpu_tracking_state.start_processing_sem);
			}
		}
		else 
		{
			int i;
			ShaderPass* shaderPass = & state->processing_step[step].shaderPass;
			for(i=0;i<nPasses;i++)
			{
				apply_shader_pass( state, shaderPass, i, &swapBuffers);
			}
			RASPITEX_FBO* finalFBO = shaderPass->fboPool[(nPasses-1)%shaderPass->fboPoolSize];
			shaderPass->finalTexture->texid = finalFBO->texture->texid;
			shaderPass->finalTexture->target = finalFBO->texture->target;
			shaderPass->finalTexture->format = finalFBO->texture->format;
		}
	}
	
	// terminate async processing cycle
	state->cpu_tracking_state.cpu_processing[ state->cpu_tracking_state.nAvailCpuFuncs ] = 0;
	vcos_semaphore_post(& state->cpu_tracking_state.start_processing_sem);

    GLCHK(glUseProgram(0));
	if(swapBuffers)
	{
		eglSwapBuffers(state->display, state->surface);
	}

    return 0;
}

int tracking_open(RASPITEX_STATE *state)
{
   state->ops.gl_init = tracking_init;
   state->ops.redraw = tracking_redraw;
   state->ops.update_texture = raspitexutil_update_texture;
   state->ops.update_y_texture = 0;
   state->ops.update_u_texture = 0;
   state->ops.update_v_texture = 0;
   return 0;
}
