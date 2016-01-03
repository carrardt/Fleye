#include "fleye/plugin.h"
#include "fleye/compiledshader.h"
#include "fleye/FleyeContext.h"

#include <iostream>

struct drawThirdHeight : public FleyePlugin
{	
	void setup(FleyeContext* ctx)
	{
	}
	
	void draw(FleyeContext* ctx ,CompiledShader* cs,int pass)
	{
		static constexpr float one_third = 2.0/3.0 - 1.0;
		static const GLfloat tstripV[12] = {
			-1, -1, 0,
			-1,  one_third, 0, 
			 1, -1, 0, 
			 1,  one_third, 0
			};

		static const GLfloat tstripT[8] = {
			0, 0,
			0, 1, 
			1, 0, 
			1, 1
			};

		cs->vertexAttrib4f(FLEYE_GL_COLOR, 1.0f,1.0f,1.0f,1.0f );
		
		cs->enableVertexArray(FLEYE_GL_VERTEX);
		cs->enableVertexArray(FLEYE_GL_TEXCOORD);
		
		cs->vertexAttribPointer(FLEYE_GL_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, tstripV);
		cs->vertexAttribPointer(FLEYE_GL_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, tstripT);
		
		GLCHK( glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
		
		cs->disableVertexArray(FLEYE_GL_TEXCOORD);
		cs->disableVertexArray(FLEYE_GL_VERTEX);
	}
};

FLEYE_REGISTER_PLUGIN(drawThirdHeight);
