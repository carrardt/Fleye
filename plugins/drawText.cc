#include "fleye/plugin.h"
#include "fleye/compiledshader.h"
#include "fleye/FleyeContext.h"

#include "services/TextService.h"

#include <iostream>

struct drawText : public FleyePlugin
{
	inline drawText() : m_arraySize(0), m_arrayIndex(0), m_varray(0), m_tarray(0) {}
	
	void setup(FleyeContext* ctx)
	{
		m_txtsvc = TextService_instance();
		m_arraySize = 1024;
		m_varray = new GLfloat[m_arraySize*8];
		m_tarray = new GLfloat[m_arraySize*8];
	}
	
	void draw(FleyeContext* ctx ,CompiledShader* cs,int pass)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		cs->enableVertexArray(FLEYE_GL_VERTEX);
		cs->enableVertexArray(FLEYE_GL_TEXCOORD);
		
		cs->vertexAttribPointer(FLEYE_GL_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, m_varray);
		cs->vertexAttribPointer(FLEYE_GL_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, m_tarray);

		// draw arbitrary positionned texts
		for( PositionnedText* pt : m_txtsvc->getPositionnedTexts() )
		{
			drawString(cs,pt->x,pt->y,pt->text);
		}
		
		// draw console text
		drawString(cs,0.02f,0.0f, m_txtsvc->consoleText() );
		
		// flush buffer
		flushArray();
		
		cs->disableVertexArray(FLEYE_GL_TEXCOORD);
		cs->disableVertexArray(FLEYE_GL_VERTEX);
		glDisable(GL_BLEND);
	}

	void drawString(CompiledShader* cs,float x, float y, std::string s)
	{
		int col=0, line=0;
		for ( char c : s )
		{
			if( c == '\n' )
			{
				++line;
				col=0;
			}
			else
			{
				if( c != ' ' )
				{
					float cx = (x+(col/(float)m_txtsvc->columns()))*2.0 - 1.0;
					float cy = 1.0 - (y+(line/(float)m_txtsvc->lines()))*2.0 ;
					//std::cout<<"cx="<<cx<<", cy="<<cy<<"\n";
					drawCharPos(cs, cx, cy, c);
				}
				++ col;
				if( col>=m_txtsvc->columns() ) { ++line; col=0; }
			}
		}
	}
	
	void flushArray()
	{
		if( m_arrayIndex > 0 )
		{
			for(int i=0;i<m_arrayIndex;i++)
			{
				glDrawArrays(GL_TRIANGLE_STRIP, i*4, 4);
			}
			m_arrayIndex = 0;
		}
	}

	void drawCharPos(CompiledShader* cs, float posX, float posY, int c)
	{
		for(int i=0;i<4;i++)
		{
			int x = i%2;
			int y = i/2;
			float vx = posX + (x ? 2.0f/m_txtsvc->columns() : 0.0f);
			float vy = posY + (y ? 2.0f/m_txtsvc->lines() : 0.0f);
			int cy = 15 - c/16;
			int cx = c%16;
			float tx = cx/16.0f + ( x ? (1.0f/16.0f)-(1.0f/256.0f) : (1.0f/256.0f) );
			float ty = cy/16.0f + ( y ? (1.0f/16.0f)-(1.0f/256.0f) : (1.0f/256.0f) );
			m_varray[m_arrayIndex*8+i*2+0] = vx;
			m_varray[m_arrayIndex*8+i*2+1] = vy;
			m_tarray[m_arrayIndex*8+i*2+0] = tx;
			m_tarray[m_arrayIndex*8+i*2+1] = ty;
		}
		
		++m_arrayIndex;
		if( m_arrayIndex == m_arraySize )
		{
			flushArray();
		}
	}

	GLfloat * m_varray;
	GLfloat * m_tarray;
	uint32_t m_arraySize;
	uint32_t m_arrayIndex;
	
	TextService* m_txtsvc;
};

FLEYE_REGISTER_PLUGIN(drawText);
