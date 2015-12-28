#ifndef fleye_CompiledShader_H_
#define fleye_CompiledShader_H_

#include "fleye/shaderprogram.h"

struct GLTexture;
struct ShaderPass;

struct CompiledShader
{
	std::vector<int> textureTargets;
	std::vector<int> samplerUniformLocations;
	ShaderProgram shader;
	
	inline void vertexAttrib2f(int attr, float x, float y)
	{
		if( shader.attribute_locations[attr] != -1 )
		{
			GLCHK( glVertexAttrib2f( shader.attribute_locations[attr], x,y ) );
		}
	}
	
	inline void vertexAttrib3f(int attr, float x, float y, float z)
	{
		if( shader.attribute_locations[attr] != -1 )
		{
			GLCHK( glVertexAttrib3f( shader.attribute_locations[attr], x,y,z ) );
		}
	}

	inline void vertexAttrib4f(int attr, float x, float y, float z, float w)
	{
		if( shader.attribute_locations[attr] != -1 )
		{
			GLCHK( glVertexAttrib4f( shader.attribute_locations[attr], x,y,z,w ) );
		}
	}

	inline void enableVertexArray(int attr)
	{
		if( shader.attribute_locations[attr] != -1 )
		{
			GLCHK( glEnableVertexAttribArray(shader.attribute_locations[attr]));
		}
	}
	inline void disableVertexArray(int attr)
	{
		if( shader.attribute_locations[attr] != -1 )
		{
			GLCHK( glDisableVertexAttribArray(shader.attribute_locations[attr]));
		}
	}
	inline void vertexAttribPointer(int attr, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
	{
		if( shader.attribute_locations[attr] != -1 )
		{
			GLCHK( glVertexAttribPointer(shader.attribute_locations[attr], size,type,normalized,stride,ptr) );
		}
	}
};

CompiledShader* get_compiled_shader(ShaderPass* shaderPass, int passCount);

#endif
