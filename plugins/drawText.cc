#include "fleye/plugin.h"
#include "fleye/compiledshader.h"
#include "fleye/FleyeContext.h"

#include "services/TextService.h"

#include <iostream>

struct drawText : public FleyePlugin
{	
	void setup(FleyeContext* ctx)
	{
		m_columns = 40;
		m_lines = 25;
		m_txtsvc = TextService_instance();
	}
	
	void draw(FleyeContext* ctx ,CompiledShader* cs,int pass)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		cs->enableVertexArray(FLEYE_GL_VERTEX);
		cs->enableVertexArray(FLEYE_GL_TEXCOORD);
		
		for( PositionnedText* pt : m_txtsvc->getPositionnedTexts() )
		{
			int col=0, line=0;
			for ( char c : pt->text )
			{
				if( c == '\n' )
				{
					++line;
					col=0;
				}
				else
				{
					float cx = (pt->x+(col/(float)m_columns))*2.0 - 1.0;
					float cy = 1.0 - (pt->y+(line/(float)m_lines))*2.0 ;
					//std::cout<<"cx="<<cx<<", cy="<<cy<<"\n";
					drawCharPos(cs, cx, cy, c);
					++ col;
					if( col>=80 ) { ++line; col=0; }
				}
			}
		}
		
		cs->enableVertexArray(FLEYE_GL_VERTEX);
		cs->disableVertexArray(FLEYE_GL_TEXCOORD);
		glDisable(GL_BLEND);
	}

	void drawCharPos(CompiledShader* cs, float posX, float posY, int c)
	{
		GLfloat varray[8];
		GLfloat tarray[8];
		for(int i=0;i<4;i++)
		{
			int x = i%2;
			int y = i/2;
			float vx = posX + (x ? 2.0f/m_columns : 0.0f);
			float vy = posY + (y ? 2.0f/m_lines : 0.0f);
			int cy = 15 - c/16;
			int cx = c%16;
			float tx = cx/16.0f + ( x ? (1.0f/16.0f)-(1.0f/256.0f) : (1.0f/256.0f) );
			float ty = cy/16.0f + ( y ? (1.0f/16.0f)-(1.0f/256.0f) : (1.0f/256.0f) );
			varray[i*2+0] = vx;
			varray[i*2+1] = vy;
			tarray[i*2+0] = tx;
			tarray[i*2+1] = ty;
		}
		cs->vertexAttribPointer(FLEYE_GL_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, varray);
		cs->vertexAttribPointer(FLEYE_GL_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, tarray);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	void drawChar(CompiledShader* cs, int col, int line, int c)
	{
		drawCharPos(cs, (2.0f*col/(float)m_columns)-1.0f, 1.0f-(2.0f*line/(float)m_lines), c );
	}

	TextService* m_txtsvc;
	int m_columns;
	int m_lines;
};

FLEYE_REGISTER_PLUGIN(drawText);
