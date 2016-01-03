#include <GLES2/gl2.h>
#include <dlfcn.h>

#include "fleye/FleyeContext.h"
#include "fleye/plugin.h"
#include "fleye/compiledshader.h"
#include "fleye/config.h"
#include <iostream>

std::map<std::string,FleyePlugin*> FleyePlugin::s_plugins;

FleyePlugin::FleyePlugin()
	: m_initialized(false)
	, m_ctx(0) {}
	
void FleyePlugin::setupOnce(FleyeContext* ctx)
{ 
	if(!m_initialized) { m_ctx=ctx; setup(ctx); m_initialized=true; }
}

std::string FleyePlugin::name() const
{
	return m_name;
}

FleyePlugin* FleyePlugin::registerPlugin(const char* name)
{ 
	m_name = name;
	s_plugins[name]=this;
	return this;
}

FleyePlugin* FleyePlugin::plugin(FleyeContext* ctx, std::string name)
{
	void * handle = NULL;
	if( name.empty() ) name = "Builtin";
	
	if( s_plugins.find(name) == s_plugins.end() )
	{
		//if(ctx->verbose) { std::cout<<"Load plugin '"<<name<<"'\n"; }
		std::string libFile = FLEYE_PLUGIN_DIR;
		libFile += "/lib" + name + ".so";
		handle = dlopen(libFile.c_str(), RTLD_GLOBAL | RTLD_NOW);		
		if(handle==NULL)
		{
			std::cerr<<"failed to load plugin "<<name<<"\n";
			abort();
		}
	}
	FleyePlugin* p = s_plugins[name];
	if( p == 0 )
	{
		std::cerr<<"Could not find plugin "<<name<<"\n";
		abort();
	}
	else
	{
		p->setupOnce(ctx);
	}
	return p;
}

// default drawing function
struct Builtin : public FleyePlugin
{
	void draw(FleyeContext* ctx, CompiledShader* compiledShader, int pass)
	{
		static const GLfloat tstripV[12] = {
			-1,-1,0,
			-1,1,0, 
			1,-1,0, 
			1,1,0
			};

		static const GLfloat tstripT[8] = {
			0,0,
			0,1, 
			1,0, 
			1,1
			};

		compiledShader->vertexAttrib4f(FLEYE_GL_COLOR, 1.0f,1.0f,1.0f,1.0f );
		
		compiledShader->enableVertexArray(FLEYE_GL_VERTEX);
		compiledShader->enableVertexArray(FLEYE_GL_TEXCOORD);
		
		compiledShader->vertexAttribPointer(FLEYE_GL_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, tstripV);
		compiledShader->vertexAttribPointer(FLEYE_GL_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, tstripT);
		
		GLCHK( glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
		
		compiledShader->disableVertexArray(FLEYE_GL_TEXCOORD);
		compiledShader->disableVertexArray(FLEYE_GL_VERTEX);
	}
};

FLEYE_REGISTER_PLUGIN(Builtin);
